#pragma once
#include <vector>
#include "Token.h"
#include "AST.h"

class Parser {
private:
    std::vector<Token> tokens;
    size_t pos = 0;

    const Token& peek() const;
    const Token& advance();
    bool match(TokenType type);
    const Token& expect(TokenType type, const std::string& what);

    // Grammar rules, one function each (recursive descent)
    StmtPtr parseStatement();
    StmtPtr parseBlock();
    ExprPtr parseExpression();     // comparison
    ExprPtr parseAdditive();       // + -
    ExprPtr parseTerm();           // * /
    ExprPtr parseFactor();         // number, variable, ( expr ), -factor
public:
    explicit Parser(std::vector<Token> t) : tokens(std::move(t)) {}
    std::unique_ptr<BlockStmt> parseProgram();
};