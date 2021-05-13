#ifndef CPPLOX_COMPILER_H
#define CPPLOX_COMPILER_H

#include "chunk.h"
#include "scanner.h"

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

    static void compile(const char* source, std::shared_ptr<Chunk> chunk);

private:

    class Parser {
        Token current;
        Token previous;
        bool hadError;
        bool panicMode;
    };

    void advance();
    void errorAtCurrent(const char* message);
    void error(const char* message);
    void errorAt(Token *token, const char* message);

    Parser parser;
}



#endif // CPPLOX_COMPILER_H