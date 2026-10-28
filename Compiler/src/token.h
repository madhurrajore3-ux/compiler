
#pragma once
#include <string>

using namespace std;

enum class TokenType {
    NUMBER, IDENTIFIER,
    LET, PRINT, IF, ELSE, WHILE,
    PLUS, MINUS, STAR, SLASH,
    ASSIGN, EQ, NEQ, LT, GT,
    LPAREN, RPAREN, LBRACE, RBRACE, SEMICOLON,
    END_OF_FILE
};

// OOP: Encapsulation. Data is private; access only through getters.
class Token {
private:
    TokenType type;
    string value;
    int line;

public:
    Token(TokenType t, const string& v, int l) : type(t), value(v), line(l) {}
    TokenType getType() const { return type; }
    const string& getValue() const { return value; }
    int getLine() const { return line; }
};
