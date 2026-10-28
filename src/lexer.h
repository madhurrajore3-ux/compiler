#pragma once
#include <string>
#include <vector>
#include "Token.h"

// OOP: Abstraction. Outside world only calls tokenize(); how it works is hidden.
class Lexer {
private:
    std::string source;
    size_t pos = 0;
    int line = 1;

    char peek() const;
    char advance();
    void skipWhitespace();
    Token readNumber();
    Token readIdentifier();
public:
    explicit Lexer(const std::string& src) : source(src) {}
    std::vector<Token> tokenize();
};