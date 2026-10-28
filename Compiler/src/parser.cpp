
#include "Parser.h"
#include "Errors.h"

using namespace std;

const Token& Parser::peek() const { return tokens[pos]; }
const Token& Parser::advance() { return tokens[pos++]; }

bool Parser::match(TokenType type) {
    if (peek().getType() == type) { pos++; return true; }
    return false;
}

const Token& Parser::expect(TokenType type, const string& what) {
    if (peek().getType() != type)
        throw ParserError("Expected " + what + " but found '" + peek().getValue() + "'", peek().getLine());
    return advance();
}

unique_ptr<BlockStmt> Parser::parseProgram() {
    auto program = make_unique<BlockStmt>();
    while (peek().getType() != TokenType::END_OF_FILE)
        program->add(parseStatement());
    return program;
}

StmtPtr Parser::parseStatement() {
    const Token& t = peek();

    if (match(TokenType::LET)) {
        string name = expect(TokenType::IDENTIFIER, "variable name").getValue();
        expect(TokenType::ASSIGN, "'='");
        ExprPtr value = parseExpression();
        expect(TokenType::SEMICOLON, "';'");
        return make_unique<AssignStmt>(name, move(value));
    }

    if (match(TokenType::PRINT)) {
        ExprPtr value = parseExpression();
        expect(TokenType::SEMICOLON, "';'");
        return make_unique<PrintStmt>(move(value));
    }

    if (match(TokenType::IF)) {
        expect(TokenType::LPAREN, "'('");
        ExprPtr cond = parseExpression();
        expect(TokenType::RPAREN, "')'");
        StmtPtr thenB = parseBlock();
        StmtPtr elseB = nullptr;
        if (match(TokenType::ELSE)) elseB = parseBlock();
        return make_unique<IfStmt>(move(cond), move(thenB), move(elseB));
    }

    if (match(TokenType::WHILE)) {
        expect(TokenType::LPAREN, "'('");
        ExprPtr cond = parseExpression();
        expect(TokenType::RPAREN, "')'");
        StmtPtr body = parseBlock();
        return make_unique<WhileStmt>(move(cond), move(body));
    }

    if (t.getType() == TokenType::IDENTIFIER) {
        string name = advance().getValue();
        expect(TokenType::ASSIGN, "'='");
        ExprPtr value = parseExpression();
        expect(TokenType::SEMICOLON, "';'");
        return make_unique<AssignStmt>(name, move(value));
    }

    throw ParserError("Unexpected token '" + t.getValue() + "'", t.getLine());
}

StmtPtr Parser::parseBlock() {
    expect(TokenType::LBRACE, "'{'");
    auto block = make_unique<BlockStmt>();

    while (peek().getType() != TokenType::RBRACE && peek().getType() != TokenType::END_OF_FILE)
        block->add(parseStatement());

    expect(TokenType::RBRACE, "'}'");
    return block;
}

// Precedence: comparison (lowest) < additive < term < factor (highest)
ExprPtr Parser::parseExpression() {
    ExprPtr left = parseAdditive();
    TokenType t = peek().getType();

    if (t == TokenType::LT || t == TokenType::GT || t == TokenType::EQ || t == TokenType::NEQ) {
        const Token& op = advance();
        ExprPtr right = parseAdditive();
        return make_unique<BinaryExpr>(move(left), op.getValue(), move(right), op.getLine());
    }

    return left;
}

ExprPtr Parser::parseAdditive() {
    ExprPtr left = parseTerm();

    while (peek().getType() == TokenType::PLUS || peek().getType() == TokenType::MINUS) {
        const Token& op = advance();
        ExprPtr right = parseTerm();
        left = make_unique<BinaryExpr>(move(left), op.getValue(), move(right), op.getLine());
    }

    return left;
}

ExprPtr Parser::parseTerm() {
    ExprPtr left = parseFactor();

    while (peek().getType() == TokenType::STAR || peek().getType() == TokenType::SLASH) {
        const Token& op = advance();
        ExprPtr right = parseFactor();
        left = make_unique<BinaryExpr>(move(left), op.getValue(), move(right), op.getLine());
    }

    return left;
}

ExprPtr Parser::parseFactor() {
    const Token& t = peek();

    if (match(TokenType::NUMBER))
        return make_unique<NumberExpr>(stoi(t.getValue()));

    if (match(TokenType::IDENTIFIER))
        return make_unique<VariableExpr>(t.getValue(), t.getLine());

    if (match(TokenType::MINUS)) {
        ExprPtr inner = parseFactor();
        return make_unique<BinaryExpr>(make_unique<NumberExpr>(0), "-", move(inner), t.getLine());
    }

    if (match(TokenType::LPAREN)) {
        ExprPtr e = parseExpression();
        expect(TokenType::RPAREN, "')'");
        return e;
    }

    throw ParserError("Unexpected token '" + t.getValue() + "'", t.getLine());
}
