#include "token.h"

Token::Token(TokenType type, const char* start, int length, int line)
: type{type}, start{start}, length{length}, line{line} {}

