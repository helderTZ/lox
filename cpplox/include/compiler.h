#ifndef CPPLOX_COMPILER_H
#define CPPLOX_COMPILER_H

#include "chunk.h"
#include "scanner.h"

#include <memory>

#define COLOR_BLACK   "\u001b[30m"
#define COLOR_RED     "\u001b[31m"
#define COLOR_GREEN   "\u001b[32m"
#define COLOR_YELLOW  "\u001b[33m"
#define COLOR_BLUE    "\u001b[34m"
#define COLOR_MAGENTA "\u001b[35m"
#define COLOR_CYAN    "\u001b[36m"
#define COLOR_WHITE   "\u001b[37m"
#define COLOR_RESET   "\u001b[0m"

class Compiler {
public:

    bool compile(const char* source, std::shared_ptr<Chunk> chunk);

private:
    void advance();
    void consume(TokenType type, const char* message);
    void emitByte(uint8_t byte);
    void emitReturn();
    Chunk* currentChunk();
    void endCompiler();

    void errorAtCurrent(const char* message);
    void error(const char* message);
    void errorAt(Token *token, const char* message);

    Token* current = nullptr;
    Token* previous = nullptr;
    bool hadError = false;
    bool panicMode = false;
    Chunk* compilingChunk;
};



#endif // CPPLOX_COMPILER_H