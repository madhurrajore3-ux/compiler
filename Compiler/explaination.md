# How Compilers Work and How This Project Works

This document explains what a compiler is, what happens when a program runs, and how every major part of this project works. It assumes only basic C++ knowledge.

## 1. Two Different Languages Are Involved

This project involves two languages, and keeping them separate avoids a lot of confusion:

1. **C++ is the implementation language.** The files in `src/` are C++ source code. `g++` turns them into the executable named `compiler`.
2. **`code` is the source language being implemented.** Files such as `tests/myprogram.code` contain programs written in the small language understood by this project.

Therefore there are two separate operations:

```text
C++ source files  --g++-->  compiler executable

myprogram.code    --compiler-->  output.tac + displayed program output
```

The first operation builds the tool. The second uses that tool.

## 2. What Is a Compiler?

A computer processor does not directly understand C++, Java, Python, or this project's `code` language. Ultimately, a real processor executes machine instructions encoded as bytes.

A **compiler** translates a program from one representation into another. A traditional native compiler might perform this pipeline:

```text
source code
   -> tokens
   -> syntax tree
   -> semantic checks
   -> intermediate representation
   -> machine code
   -> executable
```

Examples include GCC and Clang. They can translate C++ into native machine code.

An **interpreter**, by contrast, reads a program representation and performs its meaning directly. It might walk a syntax tree, evaluate expressions, store variables, and execute loops without first producing a native executable for the input program.

### What is this project exactly?

This project is a small **compiler and tree-walking interpreter**:

- It compiles the input into three-address code and writes `output.tac`.
- It also interprets the Abstract Syntax Tree (AST) directly to produce the program's output.
- It does **not** translate `.code` programs into CPU machine code.
- It does **not** execute `output.tac`; that file is currently an output for inspection only.

So when a `.code` program prints something, the AST interpreter is responsible, not the TAC file.

## 3. Building and Running the Project

From the `Compiler` directory on Linux:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic src/*.cpp -o compiler
chmod +x compiler
./compiler tests/myprogram.code
```

What each piece means:

- `g++` invokes the C++ compiler.
- `-std=c++17` selects the C++17 language standard.
- `-Wall -Wextra -pedantic` enables useful warnings.
- `src/*.cpp` selects all C++ implementation files.
- `-o compiler` names the resulting Linux executable `compiler`.
- `./compiler` runs the executable in the current directory.
- `tests/myprogram.code` becomes `argv[1]`, the input filename.

`compiler.exe` is a Windows binary. On Linux, use the native `compiler` file built by `g++`.

After editing only a `.code` file, rebuilding is unnecessary. Rebuild when the C++ files in `src/` change.

## 4. The Language Supported by This Project

The language currently supports integer variables, arithmetic, comparisons, output, conditions, and loops.

### Variables and assignment

```text
let x = 10;
let result = x + 5;
x = x - 1;
```

Both `let x = ...` and `x = ...` currently create the same `AssignStmt` AST node. Consequently, the interpreter does not enforce declaration-before-assignment and does not reject redeclaration.

### Arithmetic

```text
let answer = 2 + 3 * 4;
let grouped = (2 + 3) * 4;
let negative = -5;
```

Supported operators are `+`, `-`, `*`, and `/`. Values are C++ `int` values, so division is integer division. For example, `7 / 2` produces `3`.

### Comparisons

```text
x < 10
x > 10
x == 10
x != 10
```

A comparison evaluates to integer `1` when true and `0` when false.

### Printing

```text
print x;
print x + 10;
```

### Conditions

```text
if (x > 5) {
    print 1;
} else {
    print 0;
}
```

The `else` block is optional. Integer `0` is false; every non-zero integer is true.

### Loops

```text
while (x > 0) {
    print x;
    x = x - 1;
}
```

The condition is evaluated before every iteration.

### Important syntax rules

- Simple statements end in `;`.
- `if` and `while` conditions use parentheses.
- `if` and `while` bodies use braces.
- The language currently has integers only.
- Variable names contain letters, digits, or `_`.
- The lexer actually permits `_` as the first character too.
- Comments and strings are not currently supported.

## 5. The Language Grammar

A grammar describes which token sequences form valid programs:

```text
program     -> statement*

statement   -> "let" IDENTIFIER "=" expression ";"
             | IDENTIFIER "=" expression ";"
             | "print" expression ";"
             | "if" "(" expression ")" block ("else" block)?
             | "while" "(" expression ")" block

block       -> "{" statement* "}"

expression  -> additive (("<" | ">" | "==" | "!=") additive)?
additive    -> term (("+" | "-") term)*
term        -> factor (("*" | "/") factor)*
factor      -> NUMBER
             | IDENTIFIER
             | "(" expression ")"
             | "-" factor
```

Notation used above:

- `*` means zero or more repetitions.
- `?` means optional.
- Quoted text is a literal keyword or character.
- Uppercase names such as `NUMBER` represent token categories.

The grammar is not merely documentation: the functions in `parser.cpp` directly implement these rules.

## 6. The Complete Pipeline

For every run, `main.cpp` coordinates this pipeline:

```text
source file
   |
   v
read entire file into a C++ string
   |
   v
Lexer::tokenize()
   |
   v
vector<Token>
   |
   v
Parser::parseProgram()
   |
   v
AST rooted at BlockStmt
   |                         |
   v                         v
generate(CodeGen)        execute(SymbolTable)
   |                         |
   v                         v
output.tac               terminal output
```

Each stage has one clear responsibility. This separation is a central compiler design idea: later stages do not need to reason about individual source characters.

## 7. Stage Zero: `main.cpp`, the Driver

Execution of the C++ program begins in `main()`.

### Command-line validation

```cpp
if (argc < 2) {
    std::cerr << "Usage: compiler <file.code>\n";
    return 1;
}
```

`argc` is the number of command-line arguments. `argv` contains them. For:

```bash
./compiler tests/myprogram.code
```

the important values are approximately:

```text
argv[0] = "./compiler"
argv[1] = "tests/myprogram.code"
```

### Reading the source

`std::ifstream file(argv[1])` opens the file. A `std::stringstream` then copies the complete file into memory. `buffer.str()` is the source text passed to the lexer.

### Coordinating the stages

The essential calls are:

```cpp
Lexer lexer(buffer.str());
std::vector<Token> tokens = lexer.tokenize();

Parser parser(tokens);
std::unique_ptr<BlockStmt> program = parser.parseProgram();

CodeGen gen(out);
program->generate(gen);

SymbolTable table;
program->execute(table);
```

If any stage throws a `CompilerError`, `main()` catches it and displays its stage, line, and message.

## 8. Stage One: Lexical Analysis

The lexer converts a raw character string into meaningful tokens. This process is also called **tokenization** or **scanning**.

Given:

```text
let answer = number * 2 + 3;
```

the lexer produces conceptually:

```text
LET("let")
IDENTIFIER("answer")
ASSIGN("=")
IDENTIFIER("number")
STAR("*")
NUMBER("2")
PLUS("+")
NUMBER("3")
SEMICOLON(";")
END_OF_FILE
```

### Lexer state

The `Lexer` object stores:

- `source`: the entire input string.
- `pos`: the index of the next character.
- `line`: the current source line for error reporting.

`peek()` observes the current character without consuming it. `advance()` returns it and moves `pos` forward.

### Whitespace

`skipWhitespace()` discards spaces, tabs, and newlines. Newlines increment `line`. Newlines do not otherwise affect the language; semicolons terminate simple statements.

### Numbers

`readNumber()` consumes consecutive digits and returns a `NUMBER` token. The lexer preserves the digits as text. Later, the parser calls `std::stoi` to create an integer.

### Identifiers and keywords

`readIdentifier()` consumes letters, digits, and underscores. It then checks whether the word is a reserved keyword:

```text
let  print  if  else  while
```

If it is not a keyword, it becomes an `IDENTIFIER` token.

This keyword check explains why a variable cannot be named `while`: that text always becomes a `WHILE` token.

### Operators and punctuation

The `switch` in `tokenize()` recognizes single-character symbols. `=` and `!` require lookahead because `=` may begin `==`, while `!` is only valid as part of `!=`.

At the end, the lexer adds `END_OF_FILE`. This sentinel lets the parser know that the complete program has been consumed.

### Lexer errors

Unknown characters cause a `LexerError`:

```text
let x = 2 @ 3;
            ^
```

The lexer understands neither `@` nor comments, so it stops immediately and reports the line.

## 9. Tokens and `token.h`

A `Token` contains three pieces of information:

```cpp
TokenType type;
std::string value;
int line;
```

- `type` tells the parser the category, such as `NUMBER` or `PLUS`.
- `value` retains the source spelling, such as `"123"` or `"+"`.
- `line` supports useful diagnostics.

The fields are private and accessed through getters. This is encapsulation: callers use a stable public interface without directly changing token internals.

## 10. Stage Two: Parsing

The parser converts a flat token list into a hierarchical **Abstract Syntax Tree**.

The lexer can say that the input contains `NUMBER`, `PLUS`, and `STAR`. The parser determines their structure and meaning.

### Parser cursor helpers

- `peek()` returns the current token without consuming it.
- `advance()` consumes and returns the current token.
- `match(type)` consumes a token only if it has the requested type.
- `expect(type, description)` requires a token or throws `ParserError`.

`expect()` is why a missing semicolon can produce a focused message rather than causing mysterious behavior later.

### Recursive-descent parsing

This is a **recursive-descent parser**. Each grammar level is represented by a C++ function:

```text
grammar concept       parser function
---------------       ---------------
program               parseProgram
statement             parseStatement
block                 parseBlock
comparison            parseExpression
+ and -               parseAdditive
* and /               parseTerm
atomic expression     parseFactor
```

The functions call one another in the same shape as the grammar.

### Parsing statements

`parseStatement()` looks at the next token:

- `LET` creates an `AssignStmt`.
- `PRINT` creates a `PrintStmt`.
- `IF` creates an `IfStmt`.
- `WHILE` creates a `WhileStmt`.
- `IDENTIFIER` at statement position creates an `AssignStmt`.

`parseBlock()` consumes `{`, parses statements until `}`, and returns a `BlockStmt`.

### Operator precedence

Consider:

```text
2 + 3 * 4
```

Humans expect multiplication first, producing `14`, not `20`. The parser guarantees this by splitting precedence into levels:

```text
comparison: lowest precedence
+ -
* /
factor: highest precedence
```

`parseAdditive()` asks `parseTerm()` for each operand. Because `parseTerm()` completely consumes `3 * 4` before returning, multiplication becomes a deeper AST node and is evaluated first.

Parentheses work because `parseFactor()` sees `(`, recursively parses a complete expression, and requires `)`.

Unary minus is represented as subtraction from zero:

```text
-x  becomes  0 - x
```

### Parser errors

This invalid code:

```text
let x = 5
print x;
```

fails because the parser expects `SEMICOLON` after the expression but sees `PRINT`.

## 11. The Abstract Syntax Tree

An AST stores the program's meaning as objects connected in a tree. It is "abstract" because punctuation that has already done its job—such as semicolons and parentheses—does not need its own node.

For:

```text
let answer = number * 2 + 3;
```

the important AST shape is:

```text
AssignStmt("answer")
└── BinaryExpr("+")
    ├── BinaryExpr("*")
    │   ├── VariableExpr("number")
    │   └── NumberExpr(2)
    └── NumberExpr(3)
```

### Base classes

`ASTNode` is the common base. Two abstract subclasses define the important interfaces:

```cpp
class Expr {
    virtual int evaluate(SymbolTable&) = 0;
    virtual std::string generate(CodeGen&) = 0;
};

class Stmt {
    virtual void execute(SymbolTable&) = 0;
    virtual void generate(CodeGen&) = 0;
};
```

An **expression** produces a value. A **statement** performs an action.

### Expression nodes

- `NumberExpr` stores an integer and evaluates to it.
- `VariableExpr` looks up a name in the symbol table.
- `BinaryExpr` evaluates or generates code for two operands and an operator.

### Statement nodes

- `AssignStmt` evaluates an expression and stores its value.
- `PrintStmt` evaluates an expression and prints it.
- `BlockStmt` runs a vector of statements in order.
- `IfStmt` evaluates a condition and selects a branch.
- `WhileStmt` repeatedly evaluates a condition and executes a body.

### Ownership with `unique_ptr`

AST children use `std::unique_ptr`. A node has one clear owner, and deleting the root recursively frees the complete tree. No manual `delete` calls are needed.

`std::move` transfers ownership into a new node. After a pointer is moved, the old pointer no longer owns that object.

## 12. Stage Three A: Three-Address Code Generation

Three-address code (TAC) is a simple intermediate representation. A complex expression is broken into small instructions, usually containing one operation each.

Input:

```text
let answer = number * 2 + 3;
```

Generated output:

```text
t1 = number * 2
t2 = t1 + 3
answer = t2
```

`CodeGen::newTemp()` creates temporary names such as `t1`. `newLabel()` creates control-flow labels such as `L1`.

### Generating expressions

`BinaryExpr::generate()`:

1. Generates the left operand.
2. Generates the right operand.
3. Requests a new temporary.
4. Writes one operation into `output.tac`.
5. Returns the temporary name to its parent.

This recursive process naturally emits child calculations before their parents.

### Generating an `if`

Conceptually:

```text
t1 = answer > 10
ifFalse t1 goto L1
print 1
goto L2
L1:
print 0
L2:
```

The conditional jump skips the true block when the condition is false. The unconditional jump prevents the true path from falling into the else block.

### Generating a `while`

Conceptually:

```text
L1:
t1 = x > 0
ifFalse t1 goto L2
print x
t2 = x - 1
x = t2
goto L1
L2:
```

The backward `goto L1` creates repetition.

### What happens to `output.tac`?

It is opened with `std::ofstream out("output.tac")`. Because this is a relative path, it is created in the terminal's current working directory. Opening it this way replaces its previous contents.

No component currently reads the TAC back or executes it. A future extension could add a TAC virtual machine or translate TAC into assembly.

## 13. Stage Three B: Interpretation

After generating TAC, `main.cpp` creates a `SymbolTable` and calls:

```cpp
program->execute(table);
```

`program` is a `BlockStmt`, so it visits every top-level statement in order.

### Evaluating expressions

For `number * 2 + 3`:

1. `VariableExpr("number")` reads `5` from the symbol table.
2. `NumberExpr(2)` returns `2`.
3. The inner `BinaryExpr("*")` returns `10`.
4. `NumberExpr(3)` returns `3`.
5. The outer `BinaryExpr("+")` returns `13`.

### Executing statements

`AssignStmt` stores that `13` under `answer`. `PrintStmt` later looks it up and sends it to `std::cout`.

### Loops are real C++ loops internally

`WhileStmt::execute()` contains:

```cpp
while (condition->evaluate(table))
    body->execute(table);
```

The language's `while` behavior is implemented using C++'s `while`, but its condition and body are AST objects created from the `.code` program.

## 14. The Symbol Table

The symbol table represents program memory for named variables:

```cpp
std::map<std::string, int> variables;
```

For:

```text
let number = 5;
let answer = 13;
```

it conceptually contains:

```text
"number" -> 5
"answer" -> 13
```

`set(name, value)` inserts or replaces a value. `get(name, line)` returns it. Reading a missing name throws a `RuntimeError`.

There is currently one symbol table for the whole program. Braced blocks do not create nested scopes. A value assigned inside an `if` or `while` remains available outside it.

## 15. A Complete Traced Example

Source:

```text
let number = 5;
let answer = number * 2 + 3;
print answer;

if (answer > 10) {
    print 1;
} else {
    print 0;
}
```

### Lexer result, simplified

```text
LET IDENTIFIER ASSIGN NUMBER SEMICOLON
LET IDENTIFIER ASSIGN IDENTIFIER STAR NUMBER PLUS NUMBER SEMICOLON
PRINT IDENTIFIER SEMICOLON
IF LPAREN IDENTIFIER GT NUMBER RPAREN LBRACE
PRINT NUMBER SEMICOLON
RBRACE ELSE LBRACE
PRINT NUMBER SEMICOLON
RBRACE EOF
```

### Parser result, simplified

```text
BlockStmt
├── AssignStmt(number, NumberExpr(5))
├── AssignStmt(answer,
│   BinaryExpr(+,
│   ├── BinaryExpr(*, VariableExpr(number), NumberExpr(2))
│   └── NumberExpr(3)))
├── PrintStmt(VariableExpr(answer))
└── IfStmt
    ├── BinaryExpr(>, VariableExpr(answer), NumberExpr(10))
    ├── BlockStmt(PrintStmt(NumberExpr(1)))
    └── BlockStmt(PrintStmt(NumberExpr(0)))
```

### Generated TAC

```text
number = 5
t1 = number * 2
t2 = t1 + 3
answer = t2
print answer
t3 = answer > 10
ifFalse t3 goto L1
print 1
goto L2
L1:
print 0
L2:
```

### Interpreter state changes

```text
start                 {}
number = 5            {number: 5}
answer = 5*2+3        {number: 5, answer: 13}
print answer          displays 13
answer > 10           evaluates to 1
true block            displays 1
end                   {number: 5, answer: 13}
```

## 16. Error Handling

All project errors inherit from `CompilerError`, which itself inherits from `std::runtime_error`.

```text
CompilerError
├── LexerError
├── ParserError
└── RuntimeError
```

This lets `main()` catch all project-specific failures with one base-class reference:

```cpp
catch (const CompilerError& e)
```

The virtual `stage()` method still reports the correct derived error category. This is runtime polymorphism.

### Lexer error

```text
let x = 2 @ 3;
```

The character `@` cannot become a token.

### Syntax/parser error

```text
let x = 5
print x;
```

The token sequence violates the grammar because `;` is missing.

### Runtime error

```text
print missing;
```

The source is grammatically valid, but execution cannot find `missing` in the symbol table.

Another runtime error is division by zero:

```text
print 10 / 0;
```

## 17. Object-Oriented Programming Concepts Used

### Encapsulation

`Token`, `Lexer`, and `SymbolTable` hide internal state behind public methods.

### Abstraction

Callers can request `tokenize()`, `evaluate()`, `execute()`, or `generate()` without knowing every internal step.

### Inheritance

Concrete expression and statement classes inherit from abstract AST base classes. Error classes inherit from `CompilerError`.

### Polymorphism

An `ExprPtr` has static type `unique_ptr<Expr>`, but may point to `NumberExpr`, `VariableExpr`, or `BinaryExpr`. Calling `evaluate()` invokes the correct override at runtime.

### RAII

Streams close automatically and `unique_ptr` frees AST nodes automatically when their owners go out of scope.

## 18. What This Project Does Not Yet Do

Understanding the boundaries is just as important as understanding the implemented features:

- It does not emit native machine code.
- It does not run the generated TAC.
- It has no separate semantic-analysis/type-checking pass.
- It supports only integer values.
- It has no functions, arrays, strings, booleans, input, or comments.
- It has no lexical block scopes.
- `let` does not enforce a new declaration.
- Assigning to a previously unseen name without `let` is allowed.
- It does not detect infinite loops.
- It does not detect integer overflow.
- A very large numeric literal may cause `std::stoi` to throw a standard exception that is not formatted as a `CompilerError`.
- Only one comparison is allowed at a comparison level; chained comparisons are not part of the grammar.

These are normal limitations for a teaching compiler, not evidence that the main design is wrong.

## 19. How to Add a New Language Feature

Most features touch several stages. For example, adding remainder `%` would involve:

1. Add a token such as `PERCENT` to `TokenType`.
2. Teach the lexer to recognize `%`.
3. Teach `parseTerm()` that `%` has the same precedence as `*` and `/`.
4. Teach `BinaryExpr::evaluate()` to compute remainder and reject zero divisors.
5. TAC generation already writes a stored operator string, so it can emit `%` once parsing creates the node.
6. Add valid and invalid test programs.
7. Rebuild the C++ executable.

Adding a completely different construct, such as `for`, normally requires a token, grammar rule, parser logic, and an AST statement node with both `execute()` and `generate()` behavior.

This checklist illustrates a core compiler lesson: a language feature must have a representation and meaning at every relevant stage.

## 20. Suggested Learning Experiments

Try these in order:

1. Change constants in a `.code` file and predict the result before running it.
2. Remove a semicolon and study the syntax error.
3. Use an undefined variable and compare that runtime error with a syntax error.
4. Test precedence with `print 2 + 3 * 4;` and `print (2 + 3) * 4;`.
5. Trace a loop by hand, recording the symbol table after every iteration.
6. Inspect `output.tac` and match every temporary to an AST operation.
7. Temporarily print token types in the C++ driver to see lexical analysis directly.
8. Add `%` across the entire pipeline.
9. Add a TAC interpreter so generated code, rather than the AST, becomes executable.

## 21. File-by-File Responsibility

```text
src/main.cpp          Reads input and coordinates every stage
src/token.h           Defines token categories and token objects
src/lexer.h/.cpp      Converts characters into tokens
src/parser.h/.cpp     Converts tokens into an AST
src/ast.h             Defines AST nodes, interpretation, and TAC generation
src/symboltable.h     Stores runtime variable values
src/errors.h          Defines the compiler error hierarchy
tests/*.code          Programs written in the custom language
output.tac            Most recently generated three-address code
compiler              Native Linux executable built from the C++ files
compiler.exe          Windows executable; not the Linux build
```

## 22. Final Mental Model

When you write:

```text
print 2 + 3 * 4;
```

the project does not ask C++ or the CPU to understand that text directly. Instead:

1. The lexer recognizes words, numbers, and symbols.
2. The parser validates their order and constructs a tree.
3. The tree records that multiplication belongs below addition.
4. The code generator writes simple TAC instructions.
5. The interpreter recursively evaluates the tree.
6. `PrintStmt` sends the final integer `14` to the terminal.

That sequence—recognize, structure, represent, transform, and execute—is the central idea behind this project and behind much larger language implementations.
