#ifndef CPPLOX_SCANNER_H
#define CPPLOX_SCANNER_H

#include "token.h"

class Scanner {
public:
    Scanner(const char* source);

    Token scanToken();
    Token makeToken(TokenType type);
    Token errorToken(const char* message);
    bool isAtEnd();
    char advance();
    bool match(char expected);
    void skipWhitespace();
    char peek();
    char peekNext();
    Token string();
    Token number();
    bool isDigit(char c);
    bool isAlpha(char c);
    Token identifier();
    TokenType identifierType();
    TokenType checkKeyword(int start, int length, const char* rest, TokenType type);

    const char* start;
    const char* current;
    int line;
};

void initScanner(const char* source);

#endif // CPPLOX_SCANNER_H