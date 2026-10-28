#pragma once
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include "SymbolTable.h"
#include "Errors.h"

// Helper for code generation: hands out fresh temp names (t1, t2...) and labels (L1, L2...)
struct CodeGen {
    std::ostream& out;
    int tempCount = 0;
    int labelCount = 0;
    explicit CodeGen(std::ostream& o) : out(o) {}
    std::string newTemp()  { return "t" + std::to_string(++tempCount); }
    std::string newLabel() { return "L" + std::to_string(++labelCount); }
};

// ---------- Base classes ----------
// OOP: Abstract class with pure virtual functions. Cannot be instantiated.
class ASTNode {
public:
    virtual ~ASTNode() = default;
};

class Expr : public ASTNode {
public:
    virtual int evaluate(SymbolTable& table) = 0;            // interpreter
    virtual std::string generate(CodeGen& gen) = 0;          // compiler (returns temp/var name)
};

class Stmt : public ASTNode {
public:
    virtual void execute(SymbolTable& table) = 0;
    virtual void generate(CodeGen& gen) = 0;
};

using ExprPtr = std::unique_ptr<Expr>;   // smart pointer: memory freed automatically (RAII)
using StmtPtr = std::unique_ptr<Stmt>;

// ---------- Expressions ----------
class NumberExpr : public Expr {
    int value;
public:
    explicit NumberExpr(int v) : value(v) {}
    int evaluate(SymbolTable&) override { return value; }
    std::string generate(CodeGen&) override { return std::to_string(value); }
};

class VariableExpr : public Expr {
    std::string name; int line;
public:
    VariableExpr(const std::string& n, int l) : name(n), line(l) {}
    int evaluate(SymbolTable& table) override { return table.get(name, line); }
    std::string generate(CodeGen&) override { return name; }
};

class BinaryExpr : public Expr {
    ExprPtr left; std::string op; ExprPtr right; int line;
public:
    BinaryExpr(ExprPtr l, const std::string& o, ExprPtr r, int ln)
        : left(std::move(l)), op(o), right(std::move(r)), line(ln) {}

    // OOP: Polymorphism. left->evaluate() calls the correct derived version at runtime.
    int evaluate(SymbolTable& table) override {
        int a = left->evaluate(table);
        int b = right->evaluate(table);
        if (op == "+") return a + b;
        if (op == "-") return a - b;
        if (op == "*") return a * b;
        if (op == "/") {
            if (b == 0) throw RuntimeError("Division by zero", line);
            return a / b;
        }
        if (op == "<")  return a < b;
        if (op == ">")  return a > b;
        if (op == "==") return a == b;
        if (op == "!=") return a != b;
        throw RuntimeError("Unknown operator " + op, line);
    }

    std::string generate(CodeGen& gen) override {
        std::string a = left->generate(gen);
        std::string b = right->generate(gen);
        std::string t = gen.newTemp();
        gen.out << t << " = " << a << " " << op << " " << b << "\n";
        return t;
    }
};

// ---------- Statements ----------
class AssignStmt : public Stmt {
    std::string name; ExprPtr value;
public:
    AssignStmt(const std::string& n, ExprPtr v) : name(n), value(std::move(v)) {}
    void execute(SymbolTable& table) override { table.set(name, value->evaluate(table)); }
    void generate(CodeGen& gen) override {
        gen.out << name << " = " << value->generate(gen) << "\n";
    }
};

class PrintStmt : public Stmt {
    ExprPtr value;
public:
    explicit PrintStmt(ExprPtr v) : value(std::move(v)) {}
    void execute(SymbolTable& table) override { std::cout << value->evaluate(table) << "\n"; }
    void generate(CodeGen& gen) override { gen.out << "print " << value->generate(gen) << "\n"; }
};

class BlockStmt : public Stmt {
    std::vector<StmtPtr> statements;
public:
    void add(StmtPtr s) { statements.push_back(std::move(s)); }
    void execute(SymbolTable& table) override { for (auto& s : statements) s->execute(table); }
    void generate(CodeGen& gen) override { for (auto& s : statements) s->generate(gen); }
};

class IfStmt : public Stmt {
    ExprPtr condition; StmtPtr thenBlock; StmtPtr elseBlock;   // elseBlock may be null
public:
    IfStmt(ExprPtr c, StmtPtr t, StmtPtr e)
        : condition(std::move(c)), thenBlock(std::move(t)), elseBlock(std::move(e)) {}
    void execute(SymbolTable& table) override {
        if (condition->evaluate(table)) thenBlock->execute(table);
        else if (elseBlock) elseBlock->execute(table);
    }
    void generate(CodeGen& gen) override {
        std::string elseL = gen.newLabel(), endL = gen.newLabel();
        std::string c = condition->generate(gen);
        gen.out << "ifFalse " << c << " goto " << elseL << "\n";
        thenBlock->generate(gen);
        gen.out << "goto " << endL << "\n" << elseL << ":\n";
        if (elseBlock) elseBlock->generate(gen);
        gen.out << endL << ":\n";
    }
};

class WhileStmt : public Stmt {
    ExprPtr condition; StmtPtr body;
public:
    WhileStmt(ExprPtr c, StmtPtr b) : condition(std::move(c)), body(std::move(b)) {}
    void execute(SymbolTable& table) override {
        while (condition->evaluate(table)) body->execute(table);
    }
    void generate(CodeGen& gen) override {
        std::string startL = gen.newLabel(), endL = gen.newLabel();
        gen.out << startL << ":\n";
        std::string c = condition->generate(gen);
        gen.out << "ifFalse " << c << " goto " << endL << "\n";
        body->generate(gen);
        gen.out << "goto " << startL << "\n" << endL << ":\n";
    }
};