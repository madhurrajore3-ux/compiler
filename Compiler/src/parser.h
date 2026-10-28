
#pragma once
#include <vector>
#include "Token.h"
#include "AST.h"

using namespace std;

class Parser {
private:
    vector<Token> tokens;
    size_t pos = 0;

    const Token& peek() const;
    const Token& advance() const;
    bool match(TokenType type);
    const Token& expect(TokenType type, const string& what);

    // Grammar rules, one function each (recursive descent)
    StmtPtr parseStatement();
    StmtPtr parseBlock();
    ExprPtr parseExpression();     // comparison
    ExprPtr parseAdditive();       // + -
    ExprPtr parseTerm();           // * /
    ExprPtr parseFactor();         // number, variable, ( expr ), -factor

public:
    explicit Parser(vector<Token> t) : tokens(move(t)) {}
    unique_ptr<BlockStmt> parseProgram();
};
