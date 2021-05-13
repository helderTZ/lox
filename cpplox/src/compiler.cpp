#include "compiler.h"
#include "scanner.h"

#include <cstdio>

bool Compiler::compile(const char* source, std::shared_ptr<Chunk> chunk) {
    Scanner scanner(source);

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
    previous = current;

    for(;;) {
        current = scanToken();
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

void Compiler::emitByte(uint8_t byte) {
    currentChunk()->writeChunk(byte, previous->line);
}

Chunk* Compiler::currentChunk() {
    return compilingChunk;
}

void Compiler::endCompiler() {
    emitReturn();
}

void emitReturn() {
    emitByte(OP_RETURN);
}

void Compiler::errorAtCurrent(const char* message) {
    errorAt(current, message)
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