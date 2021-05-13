#include "compiler.h"
#include "scanner.h"

#include <cstdio>

void Compiler::compile(const char* source, std::shared_ptr<Chunk> chunk) {
    Scanner scanner(source);

    parser.hadError = false;
    parser.panicMode = false;

    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");

    return !parser.hadError;
    
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
    parser.previous = parser.current;

    for(;;) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR)
            break;

        errorAtCurrent(parser.current.start);
    }
}

void Compiler::errorAtCurrent(const char* message) {
    errorAt(&parser.current, message)
}

void Compiler::error(const char* message) {
    errorAt(&parser.previous, message);
}

void Compiler::errorAt(Token* token, const char* message) {
    if(parser.panicMode)
        return;

    parser.panicMode = true;
    fprintf(stderr, COLOR_RED "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {

    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s" COLOR_RESET "\n", message);
    parser.hadError = true;
}