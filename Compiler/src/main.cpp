
#include <iostream>
#include <fstream>
#include <sstream>
#include "Lexer.h"
#include "Parser.h"
#include "AST.h"
#include "Errors.h"

using namespace std;

// code Compiler
// Pipeline: Source (.code) -> Lexer -> Tokens -> Parser -> AST -> Code Generator / Interpreter

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: compiler <file.code>\n";
        return 1;
    }

    ifstream file(argv[1]);
    if (!file) {
        cerr << "Cannot open " << argv[1] << "\n";
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();

    cout << "=== code Compiler ===\n";

    try {
        // Stage 1: Lexical analysis (source text -> tokens)
        Lexer lexer(buffer.str());
        vector<Token> tokens = lexer.tokenize();

        // Stage 2: Parsing (tokens -> AST)
        Parser parser(tokens);
        unique_ptr<BlockStmt> program = parser.parseProgram();

        // Stage 3a: Code generation (AST -> three-address code)
        ofstream out("output.tac");
        CodeGen gen(out);
        program->generate(gen);
        cout << "[Compiled to output.tac]\n";

        // Stage 3b: Interpretation (run the program)
        cout << "[Program output]\n";
        SymbolTable table;
        program->execute(table);
    }
    // OOP: catching by base-class reference; stage() is resolved polymorphically
    catch (const CompilerError& e) {
        cerr << e.stage() << " (line " << e.getLine() << "): " << e.what() << "\n";
        return 1;
    }

    return 0;
}
