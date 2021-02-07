#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
  Token current;
  Token previous;
  bool hadError;
  bool panicMode;
} Parser;

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_CONDITIONAL, // ?:
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool canAssign);

typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;

typedef struct {
  Token name;
  int depth;
} Local;

typedef struct {
  Local locals[UINT8_COUNT];
  int localCount;
  int scopeDepth;
} Compiler;

Parser parser;

Compiler* current = NULL;

Chunk* compilingChunk;

static Chunk* currentChunk() {
  return compilingChunk;
}

static void errorAt(Token* token, const char* message) {
  if (parser.panicMode) return;
  parser.panicMode = true;

  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing.
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  parser.hadError = true;
}

static void error(const char* message) {
  errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char* message) {
  errorAt(&parser.current, message);
}

static void advance() {
  parser.previous = parser.current;

  for (;;) {
    parser.current = scanToken();
    if (parser.current.type != TOKEN_ERROR) break;

    errorAtCurrent(parser.current.start);
  }
}

static void consume(TokenType type, const char* message) {
  if (parser.current.type == type) {
    advance();
    return;
  }
  errorAtCurrent(message);
}

static bool check(TokenType type) {
  return parser.current.type == type;
}

static bool match(TokenType type) {
  if (!check(type)) return false;
  advance();
  return true;
}

static void emitByte(uint8_t byte) {
  writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
  emitByte(byte1);
  emitByte(byte2);
}

static void emitBytesLong(uint8_t byte1, uint32_t bytes234) {
  emitByte(byte1);
  emitByte((uint8_t)(bytes234 >> 16 & 0xff));
  emitByte((uint8_t)(bytes234 >> 8  & 0xff));
  emitByte((uint8_t)(bytes234       & 0xff));
}

static void emitReturn() {
  emitByte(OP_RETURN);
}

static uint8_t makeConstant(Value value) {
  int index = addConstant(currentChunk(), value);
  return (uint8_t)index;
}

static uint32_t makeLongConstant(Value value) {
  int index = addConstant(currentChunk(), value);
  return (uint32_t)index;
}

static void emitConstant(Value value) {
  if (currentChunk()->constants.count > 255) {
    emitBytesLong(OP_CONSTANT_LONG, makeLongConstant(value));
  } else
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void initCompiler(Compiler* compiler) {
  compiler->localCount = 0;
  compiler->scopeDepth = 0;
  current = compiler;
}

static void endCompiler() {
  emitReturn();
#ifdef DEBUG_PRINT_CODE
  if (!parser.hadError) {
    disassembleChunk(currentChunk(), "code");
  }
#endif
}

static void beginScope() {
  current->scopeDepth++;
}

static void endScope() {
  current->scopeDepth--;

  while (current->localCount > 0 &&
         current->locals[current->localCount - 1].depth >
            current->scopeDepth) {
    emitByte(OP_POP);
    current->localCount--;
  }
}

// forward declarations
static void expression();
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);
bool compile(const char* source, Chunk* chunk);
static uint8_t identifierConstant(Token* name);
static int resolveLocal(Compiler* compiler, Token* name);

static void conditional(bool canAssign) {
  // // Compile the then branch.
  // parsePrecedence(compiler, PREC_CONDITIONAL);

  // consume(compiler, TOKEN_COLON,
  //     "Expect ':' after then branch of conditional operator.");

  // // Compile the else branch.
  // parsePrecedence(compiler, PREC_ASSIGNMENT);
}

static void binary(bool canAssign) {
  // Remember the operator.
  TokenType operatorType = parser.previous.type;

  // Compile the right operand.
  ParseRule* rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1));

  // Emit the operator instruction.
  switch (operatorType) {
    case TOKEN_BANG_EQUAL:    emitBytes(OP_EQUAL, OP_NOT); break;
    case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL); break;
    case TOKEN_GREATER:       emitByte(OP_GREATER); break;
    case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
    case TOKEN_LESS:          emitByte(OP_LESS); break;
    case TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT); break;
    case TOKEN_PLUS:          emitByte(OP_ADD); break;
    case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
    case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
    case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
    default:
      return; // Unreachable.
  }
}

static void literal(bool canAssign) {
  switch (parser.previous.type) {
    case TOKEN_FALSE: emitByte(OP_FALSE); break;
    case TOKEN_NIL:   emitByte(OP_NIL); break;
    case TOKEN_TRUE:  emitByte(OP_TRUE); break;
    default:
      return; // Unreachable.
  }
}

static void grouping(bool canAssign) {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(bool canAssign) {
  double value = strtod(parser.previous.start, NULL);
  if (value == 0) { emitByte(OP_ZERO); return; }
  if (value == 1) { emitByte(OP_ONE); return; }
  emitConstant(NUMBER_VAL(value));
}

static void string(bool canAssign) {
  emitConstant(OBJ_VAL(copyString(parser.previous.start + 1,
                                  parser.previous.length - 2)));
}

static void namedVariable(Token name, bool canAssign) {
  uint8_t getOp, setOp;
  int arg = resolveLocal(current, &name);
  if (arg != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  } else {
    arg = identifierConstant(&name);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();
    emitBytes(setOp, (uint8_t)arg);
  } else {
    emitBytes(getOp, (uint8_t)arg);
  }
}

static void variable(bool canAssign) {
  namedVariable(parser.previous, canAssign);
}

static void unary(bool canAssign) {
  TokenType operatorType = parser.previous.type;

  // Compile the operand.
  parsePrecedence(PREC_UNARY);

  // Emit the operator instruction.
  switch (operatorType) {
    case TOKEN_BANG: emitByte(OP_NOT); break;
    case TOKEN_MINUS: emitByte(OP_NEGATE); break;
    default:
      return; // Unreachable.
  }
}

ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping,    NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,        NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,        NULL,   PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,        NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,        NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,        NULL,   PREC_NONE},
  [TOKEN_MINUS]         = {unary,       binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,        binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,        NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,        binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,        binary, PREC_FACTOR},
  [TOKEN_INTERROGATION] = {conditional, NULL,   PREC_NONE},
  [TOKEN_COLON]         = {NULL,        NULL,   PREC_NONE},
  [TOKEN_BANG]          = {unary,       NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,        binary, PREC_EQUALITY},
  [TOKEN_EQUAL]         = {NULL,        NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,        binary, PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,        binary, PREC_COMPARISON},
  [TOKEN_GREATER_EQUAL] = {NULL,        binary, PREC_COMPARISON},
  [TOKEN_LESS]          = {NULL,        binary, PREC_COMPARISON},
  [TOKEN_LESS_EQUAL]    = {NULL,        binary, PREC_COMPARISON},
  [TOKEN_IDENTIFIER]    = {variable,    NULL,   PREC_NONE},
  [TOKEN_STRING]        = {string,      NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,      NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,        NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,        NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,        NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,        NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,        NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,        NULL,   PREC_NONE},
  [TOKEN_NIL]           = {literal,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,        NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,        NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,        NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,        NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,        NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {literal,     NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,        NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,        NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,        NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,        NULL,   PREC_NONE},
};

static void parsePrecedence(Precedence precedence) {
  advance();
  ParseFn prefixRule = getRule(parser.previous.type)->prefix;
  if (prefixRule == NULL) {
    error("Expect expression.");
    return;
  }

  bool canAssign = precedence <= PREC_ASSIGNMENT;
  prefixRule(canAssign);

  while (precedence <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infixRule = getRule(parser.previous.type)->infix;
    infixRule(canAssign);
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    error("Invalid assignment target.");
  }
}

static uint8_t identifierConstant(Token* name) {
  return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

static bool identifiersEqual(Token* a, Token* b) {
  if (a->length != b->length) return false;
  return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Compiler* compiler, Token* name) {
  for (int i = compiler->localCount - 1; i >= 0; i--) {
    Local* local = &compiler->locals[i];
    if (identifiersEqual(name, &local->name)) {
      if (local->depth == -1) {
        error("Can't read local variable in its own initializer.");
      }
      return i;
    }
  }

  return -1;
}

static void addLocal(Token name) {
  if (current->localCount == UINT8_COUNT) {
    error("Too many local variables in function.");
    return;
  }
  Local* local = &current->locals[current->localCount++];
  local->name = name;
  local->depth = -1;
}

static void declareVariable() {
  if (current->scopeDepth == 0) return;

  Token* name = &parser.previous;
  for (int i = current->localCount - 1; i >= 0; i--) {
    Local* local = &current->locals[i];
    if (local->depth != -1 && local->depth < current->scopeDepth) {
      break; 
    }

    if (identifiersEqual(name, &local->name)) {
      error("Already variable with this name in this scope.");
    }
  }

  addLocal(*name);
}

static uint8_t parseVariable(const char* errorMessage) {
  consume(TOKEN_IDENTIFIER, errorMessage);

  declareVariable();
  if (current->scopeDepth > 0) return 0;

  return identifierConstant(&parser.previous);
}

static void markInitialized() {
  current->locals[current->localCount - 1].depth =
      current->scopeDepth;
}

static void defineVariable(uint8_t global) {
  if (current->scopeDepth > 0) {
    markInitialized();
    return;
  }

  emitBytes(OP_DEFINE_GLOBAL, global);
}

static ParseRule* getRule(TokenType type) {
  return &rules[type];
}

static void expression() {
  parsePrecedence(PREC_ASSIGNMENT);
}

static void block() {
  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    declaration();
  }

  consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void varDeclaration() {
  uint8_t global = parseVariable("Expect variable name.");

  if (match(TOKEN_EQUAL)) {
    expression();
  } else {
    emitByte(OP_NIL);
  }
  consume(TOKEN_SEMICOLON,
          "Expect ';' after variable declaration.");

  defineVariable(global);
}

static void expressionStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
  emitByte(OP_POP);
}

static void printStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after value.");
  emitByte(OP_PRINT);
}

static void synchronize() {
  parser.panicMode = false;

  while (parser.current.type != TOKEN_EOF) {
    if (parser.previous.type == TOKEN_SEMICOLON) return;

    switch (parser.current.type) {
      case TOKEN_CLASS:
      case TOKEN_FUN:
      case TOKEN_VAR:
      case TOKEN_FOR:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_PRINT:
      case TOKEN_RETURN:
        return;

      default:
        // Do nothing.
        ;
    }

    advance();
  }
}

static void declaration() {
  if (match(TOKEN_VAR)) {
    varDeclaration();
  } else {
    statement();
  }

  if (parser.panicMode) synchronize();
}

static void statement() {
  if (match(TOKEN_PRINT)) {
    printStatement();
  } else if (match(TOKEN_LEFT_BRACE)) {
    beginScope();
    block();
    endScope();
  } else {
    expressionStatement();
  }
}

bool compile(const char* source, Chunk* chunk) {
  initScanner(source);
  Compiler compiler;
  initCompiler(&compiler);
  compilingChunk = chunk;

  parser.hadError = false;
  parser.panicMode = false;

  advance();

  while (!match(TOKEN_EOF)) {
    declaration();
  }

  endCompiler();

  return !parser.hadError;
}