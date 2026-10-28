
#pragma once
#include <string>
#include <vector>
#include "Token.h"

using namespace std;

// OOP: Abstraction. Outside world only calls tokenize(); how it works is hidden.
class Lexer {
private:
    string source;
    size_t pos = 0;
    int line = 1;

    char peek() const;
    char advance();
    void skipWhitespace();
    Token readNumber();
    Token readIdentifier();

public:
    explicit Lexer(const string& src) : source(src) {}
    vector<Token> tokenize();
};
