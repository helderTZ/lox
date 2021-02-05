#include <stdio.h>
#include <stdlib.h>

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

typedef void (*ParseFn)();

typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;

Parser parser;

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

static void endCompiler() {
  emitReturn();
#ifdef DEBUG_PRINT_CODE
  if (!parser.hadError) {
    disassembleChunk(currentChunk(), "code");
  }
#endif
}

static void expression();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);
bool compile(const char* source, Chunk* chunk);

static void conditional()
{
  // // Compile the then branch.
  // parsePrecedence(compiler, PREC_CONDITIONAL);

  // consume(compiler, TOKEN_COLON,
  //     "Expect ':' after then branch of conditional operator.");

  // // Compile the else branch.
  // parsePrecedence(compiler, PREC_ASSIGNMENT);
}

static void binary() {
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

static void literal() {
  switch (parser.previous.type) {
    case TOKEN_FALSE: emitByte(OP_FALSE); break;
    case TOKEN_NIL:   emitByte(OP_NIL); break;
    case TOKEN_TRUE:  emitByte(OP_TRUE); break;
    default:
      return; // Unreachable.
  }
}

static void grouping() {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number() {
  double value = strtod(parser.previous.start, NULL);
  if (value == 0) { emitByte(OP_ZERO); return; }
  if (value == 1) { emitByte(OP_ONE); return; }
  emitConstant(NUMBER_VAL(value));
}

static void string() {
  emitConstant(OBJ_VAL(copyString(parser.previous.start + 1,
                                  parser.previous.length - 2)));
}

static void unary() {
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
  [TOKEN_IDENTIFIER]    = {NULL,        NULL,   PREC_NONE},
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

  prefixRule();

  while (precedence <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infixRule = getRule(parser.previous.type)->infix;
    infixRule();
  }
}

static ParseRule* getRule(TokenType type) {
  return &rules[type];
}

static void expression() {
  parsePrecedence(PREC_ASSIGNMENT);
}

bool compile(const char* source, Chunk* chunk) {
  initScanner(source);
  compilingChunk = chunk;

  parser.hadError = false;
  parser.panicMode = false;

  advance();
  expression();
  consume(TOKEN_EOF, "Expect end of expression.");
  endCompiler();

  return !parser.hadError;
}