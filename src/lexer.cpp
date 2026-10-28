#include "Lexer.h"
#include "Errors.h"
#include <cctype>

char Lexer::peek() const { return pos < source.size() ? source[pos] : '\0'; }
char Lexer::advance() { return source[pos++]; }

void Lexer::skipWhitespace() {
    while (std::isspace(peek())) {
        if (peek() == '\n') line++;
        advance();
    }
}

Token Lexer::readNumber() {
    std::string num;
    while (std::isdigit(peek())) num += advance();
    return Token(TokenType::NUMBER, num, line);
}

Token Lexer::readIdentifier() {
    std::string word;
    while (std::isalnum(peek()) || peek() == '_') word += advance();
    if (word == "let")   return Token(TokenType::LET, word, line);
    if (word == "print") return Token(TokenType::PRINT, word, line);
    if (word == "if")    return Token(TokenType::IF, word, line);
    if (word == "else")  return Token(TokenType::ELSE, word, line);
    if (word == "while") return Token(TokenType::WHILE, word, line);
    return Token(TokenType::IDENTIFIER, word, line);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        skipWhitespace();
        char c = peek();
        if (c == '\0') break;

        if (std::isdigit(c)) { tokens.push_back(readNumber()); continue; }
        if (std::isalpha(c) || c == '_') { tokens.push_back(readIdentifier()); continue; }

        advance();
        switch (c) {
            case '+': tokens.emplace_back(TokenType::PLUS, "+", line); break;
            case '-': tokens.emplace_back(TokenType::MINUS, "-", line); break;
            case '*': tokens.emplace_back(TokenType::STAR, "*", line); break;
            case '/': tokens.emplace_back(TokenType::SLASH, "/", line); break;
            case '(': tokens.emplace_back(TokenType::LPAREN, "(", line); break;
            case ')': tokens.emplace_back(TokenType::RPAREN, ")", line); break;
            case '{': tokens.emplace_back(TokenType::LBRACE, "{", line); break;
            case '}': tokens.emplace_back(TokenType::RBRACE, "}", line); break;
            case ';': tokens.emplace_back(TokenType::SEMICOLON, ";", line); break;
            case '<': tokens.emplace_back(TokenType::LT, "<", line); break;
            case '>': tokens.emplace_back(TokenType::GT, ">", line); break;
            case '=':
                if (peek() == '=') { advance(); tokens.emplace_back(TokenType::EQ, "==", line); }
                else tokens.emplace_back(TokenType::ASSIGN, "=", line);
                break;
            case '!':
                if (peek() == '=') { advance(); tokens.emplace_back(TokenType::NEQ, "!=", line); }
                else throw LexerError("Unexpected character '!'", line);
                break;
            default:
                throw LexerError(std::string("Unexpected character '") + c + "'", line);
        }
    }
    tokens.emplace_back(TokenType::END_OF_FILE, "", line);
    return tokens;
}