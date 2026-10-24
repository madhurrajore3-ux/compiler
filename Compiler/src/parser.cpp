#include "Parser.h"
#include "Errors.h"

const Token& Parser::peek() const { return tokens[pos]; }
const Token& Parser::advance() { return tokens[pos++]; }

bool Parser::match(TokenType type) {
    if (peek().getType() == type) { pos++; return true; }
    return false;
}

const Token& Parser::expect(TokenType type, const std::string& what) {
    if (peek().getType() != type)
        throw ParserError("Expected " + what + " but found '" + peek().getValue() + "'", peek().getLine());
    return advance();
}

std::unique_ptr<BlockStmt> Parser::parseProgram() {
    auto program = std::make_unique<BlockStmt>();
    while (peek().getType() != TokenType::END_OF_FILE)
        program->add(parseStatement());
    return program;
}

StmtPtr Parser::parseStatement() {
    const Token& t = peek();

    if (match(TokenType::LET)) {
        std::string name = expect(TokenType::IDENTIFIER, "variable name").getValue();
        expect(TokenType::ASSIGN, "'='");
        ExprPtr value = parseExpression();
        expect(TokenType::SEMICOLON, "';'");
        return std::make_unique<AssignStmt>(name, std::move(value));
    }
    if (match(TokenType::PRINT)) {
        ExprPtr value = parseExpression();
        expect(TokenType::SEMICOLON, "';'");
        return std::make_unique<PrintStmt>(std::move(value));
    }
    if (match(TokenType::IF)) {
        expect(TokenType::LPAREN, "'('");
        ExprPtr cond = parseExpression();
        expect(TokenType::RPAREN, "')'");
        StmtPtr thenB = parseBlock();
        StmtPtr elseB = nullptr;
        if (match(TokenType::ELSE)) elseB = parseBlock();
        return std::make_unique<IfStmt>(std::move(cond), std::move(thenB), std::move(elseB));
    }
    if (match(TokenType::WHILE)) {
        expect(TokenType::LPAREN, "'('");
        ExprPtr cond = parseExpression();
        expect(TokenType::RPAREN, "')'");
        StmtPtr body = parseBlock();
        return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
    }
    if (t.getType() == TokenType::IDENTIFIER) {          // x = expr;
        std::string name = advance().getValue();
        expect(TokenType::ASSIGN, "'='");
        ExprPtr value = parseExpression();
        expect(TokenType::SEMICOLON, "';'");
        return std::make_unique<AssignStmt>(name, std::move(value));
    }
    throw ParserError("Unexpected token '" + t.getValue() + "'", t.getLine());
}

StmtPtr Parser::parseBlock() {
    expect(TokenType::LBRACE, "'{'");
    auto block = std::make_unique<BlockStmt>();
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
        return std::make_unique<BinaryExpr>(std::move(left), op.getValue(), std::move(right), op.getLine());
    }
    return left;
}

ExprPtr Parser::parseAdditive() {
    ExprPtr left = parseTerm();
    while (peek().getType() == TokenType::PLUS || peek().getType() == TokenType::MINUS) {
        const Token& op = advance();
        ExprPtr right = parseTerm();
        left = std::make_unique<BinaryExpr>(std::move(left), op.getValue(), std::move(right), op.getLine());
    }
    return left;
}

ExprPtr Parser::parseTerm() {
    ExprPtr left = parseFactor();
    while (peek().getType() == TokenType::STAR || peek().getType() == TokenType::SLASH) {
        const Token& op = advance();
        ExprPtr right = parseFactor();
        left = std::make_unique<BinaryExpr>(std::move(left), op.getValue(), std::move(right), op.getLine());
    }
    return left;
}

ExprPtr Parser::parseFactor() {
    const Token& t = peek();
    if (match(TokenType::NUMBER))     return std::make_unique<NumberExpr>(std::stoi(t.getValue()));
    if (match(TokenType::IDENTIFIER)) return std::make_unique<VariableExpr>(t.getValue(), t.getLine());
    if (match(TokenType::MINUS)) {    // unary minus: 0 - factor
        ExprPtr inner = parseFactor();
        return std::make_unique<BinaryExpr>(std::make_unique<NumberExpr>(0), "-", std::move(inner), t.getLine());
    }
    if (match(TokenType::LPAREN)) {
        ExprPtr e = parseExpression();
        expect(TokenType::RPAREN, "')'");
        return e;
    }
    throw ParserError("Unexpected token '" + t.getValue() + "'", t.getLine());
}