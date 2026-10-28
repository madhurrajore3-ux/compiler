#pragma once
#include <stdexcept>
#include <string>

// OOP: Inheritance from std::runtime_error, and a base class for our own hierarchy.
class CompilerError : public std::runtime_error {
    int line;
public:
    CompilerError(const std::string& msg, int l) : std::runtime_error(msg), line(l) {}
    int getLine() const { return line; }
    virtual std::string stage() const { return "Error"; }   // overridden below
};

class LexerError : public CompilerError {
public:
    using CompilerError::CompilerError;
    std::string stage() const override { return "Lexer Error"; }
};

class ParserError : public CompilerError {
public:
    using CompilerError::CompilerError;
    std::string stage() const override { return "Syntax Error"; }
};

class RuntimeError : public CompilerError {
public:
    using CompilerError::CompilerError;
    std::string stage() const override { return "Runtime Error"; }
};