#ifndef CPPLOX_COMPILER_H
#define CPPLOX_COMPILER_H

#include "chunk.h"
#include "scanner.h"

#include <memory>
#include <unordered_map>

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
private:
    Token* current = nullptr;
    Token* previous = nullptr;
    bool hadError = false;
    bool panicMode = false;
    std::shared_ptr<Scanner> scanner;
    Chunk* compilingChunk;

public:
    bool compile(const char* source, std::shared_ptr<Chunk> chunk);

private:
    void advance();
    void consume(TokenType type, const char* message);
    void endCompiler();
    Chunk* currentChunk();

    // ----------------------------------
    //      Emit OpCodes methods
    // ----------------------------------
    void emitByte(uint8_t byte);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    void emitReturn();
    void emitConstant(Value value);
    uint8_t makeConstant(Value value);

    // ----------------------------------
    //      Parsing methods
    // ----------------------------------
    typedef enum Precedence : uint8_t {
        PREC_NONE,
        PREC_ASSIGNMENT,  // =
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

    class ParseRule {
    public:
        using ParseFn = void (Compiler::*)();
        ParseFn prefix;
        ParseFn infix;
        Precedence precedence;
    };

    std::unordered_map<TokenType, ParseRule> rules = {
        {TOKEN_LEFT_PAREN     , {&Compiler::grouping,   NULL,               PREC_NONE}   } ,
        {TOKEN_RIGHT_PAREN    , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_LEFT_BRACE     , {NULL,                  NULL,               PREC_NONE}   } , 
        {TOKEN_RIGHT_BRACE    , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_COMMA          , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_DOT            , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_MINUS          , {&Compiler::unary,      &Compiler::binary,  PREC_TERM}   } ,
        {TOKEN_PLUS           , {NULL,                  &Compiler::binary,  PREC_TERM}   } ,
        {TOKEN_SEMICOLON      , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_SLASH          , {NULL,                  &Compiler::binary,  PREC_FACTOR} } ,
        {TOKEN_STAR           , {NULL,                  &Compiler::binary,  PREC_FACTOR} } ,
        {TOKEN_BANG           , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_BANG_EQUAL     , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_EQUAL          , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_EQUAL_EQUAL    , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_GREATER        , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_GREATER_EQUAL  , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_LESS           , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_LESS_EQUAL     , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_IDENTIFIER     , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_STRING         , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_NUMBER         , {&Compiler::number,     NULL,               PREC_NONE}   } ,
        {TOKEN_AND            , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_CLASS          , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_ELSE           , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_FALSE          , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_FOR            , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_FUN            , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_IF             , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_NIL            , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_OR             , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_PRINT          , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_RETURN         , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_SUPER          , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_THIS           , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_TRUE           , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_VAR            , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_WHILE          , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_ERROR          , {NULL,                  NULL,               PREC_NONE}   } ,
        {TOKEN_EOF            , {NULL,                  NULL,               PREC_NONE}   }
    };

    void parsePrecedence(Precedence precedence);
    ParseRule* getRule(TokenType type);
    void expression();
    void number();
    void grouping();
    void unary();
    void binary();

    // ----------------------------------
    //      Error methods
    // ----------------------------------
    void errorAtCurrent(const char* message);
    void error(const char* message);
    void errorAt(Token *token, const char* message);
};




#endif // CPPLOX_COMPILER_H