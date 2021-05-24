#include "compiler.h"
#include "scanner.h"

#include <cstdio>
#include <new>

bool Compiler::compile(const char* source, std::shared_ptr<Chunk> chunk) {
    scanner = std::make_shared<Scanner>(source);

    hadError = false;
    panicMode = false;

    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");
    endCompiler();

    return !hadError;
    
    // int line = -1;
    // for (;;) {
    //     Token token = scanToken();
    //     if (token.line != line) {
    //         printf("%4d ", token.line);
    //         line = token.line;
    //     } else {
    //         printf("   | ");
    //     }
    //     printf("%2d '%.*s'\n", token.type, token.length, token.start); 

    //     if (token.type == TOKEN_EOF) break;
    // }
}

void Compiler::advance() {
    // if (current != nullptr)
        previous = current;

    for(;;) {
        current = &(scanner->scanToken());
        if (current->type != TOKEN_ERROR)
            break;

        errorAtCurrent(current->start);
    }
}

void Compiler::consume(TokenType type, const char* message) {
    if (current->type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

void Compiler::endCompiler() {
    emitReturn();
}

// ----------------------------------
//      Emit OpCodes methods
// ----------------------------------

void Compiler::emitByte(uint8_t byte) {
    currentChunk()->writeChunk(byte, previous->line);
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

void Compiler::emitReturn() {
    emitByte(OP_RETURN);
}

void Compiler::emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

uint8_t Compiler::makeConstant(Value value) {
    int constant = currentChunk()->addConstant(value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t) constant;
}

Chunk* Compiler::currentChunk() {
    return compilingChunk;
}

// ----------------------------------
//      Parsing methods
// ----------------------------------

void Compiler::parsePrecedence(Precedence precedence) {

}

Compiler::ParseRule* Compiler::getRule(TokenType type) {
    return &rules[type];
}

void Compiler::expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

void Compiler::number() {
    double value = strtod(previous->start, NULL);
    emitConstant(value);
}

void Compiler::grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' adter expression.");
}

void Compiler::unary() {
    TokenType operatorType = previous->type;

    // Compile the operand
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction
    switch (operatorType) {
        case TOKEN_MINUS: 
            emitByte(OP_NEGATE);
            break;
        default:
            return; // Unreachable
    }
}

void Compiler::binary() {
    TokenType operatorType = previous->type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (operatorType) {
        case TOKEN_PLUS:    emitByte(OP_ADD); break;
        case TOKEN_MINUS:   emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR:    emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH:   emitByte(OP_DIVIDE); break;
        default: return; // Unreachable
    }
}

// ----------------------------------
//      Error methods
// ----------------------------------

void Compiler::errorAtCurrent(const char* message) {
    errorAt(current, message);
}

void Compiler::error(const char* message) {
    errorAt(previous, message);
}

void Compiler::errorAt(Token* token, const char* message) {
    if(panicMode)
        return;

    panicMode = true;
    fprintf(stderr, COLOR_RED "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {

    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s" COLOR_RESET "\n", message);
    hadError = true;
}