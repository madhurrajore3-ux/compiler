
#pragma once
#include <stdexcept>
#include <string>

using namespace std;

// OOP: Inheritance from runtime_error, and a base class for our own hierarchy.
class CompilerError : public runtime_error {
    int line;
public:
    CompilerError(const string& msg, int l) : runtime_error(msg), line(l) {}
    int getLine() const { return line; }
    virtual string stage() const { return "Error"; }
};

class LexerError : public CompilerError {
public:
    using CompilerError::CompilerError;
    string stage() const override { return "Lexer Error"; }
};

class ParserError : public CompilerError {
public:
    using CompilerError::CompilerError;
    string stage() const override { return "Syntax Error"; }
};

class RuntimeError : public CompilerError {
public:
    using CompilerError::CompilerError;
    string stage() const override { return "Runtime Error"; }
};
