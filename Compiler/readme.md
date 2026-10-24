# code Compiler

A compiler and interpreter for **code**, a minimal imperative language, written in C++17.
Built as an Object-Oriented Programming project to demonstrate classes, inheritance,
polymorphism, encapsulation, abstraction and exception handling in a real system.

## Features

- Integer variables with `let` and reassignment
- Arithmetic: `+ - * /`, parentheses, unary minus, correct operator precedence
- Comparisons: `< > == !=`
- `print` statement
- `if / else` and `while` with nested blocks
- Compiles source to three-address code (`output.tac`)
- Interprets and runs the program directly
- Error reporting with stage name and line number (lexer, syntax, runtime)

## Project Structure

```
Compiler/
├── src/
│   ├── token.h          Token class and TokenType enum
│   ├── errors.h         Exception hierarchy (CompilerError → Lexer/Parser/Runtime)
│   ├── lexer.h/.cpp     Lexical analysis: source text → tokens
│   ├── ast.h            Abstract Syntax Tree node classes
│   ├── symboltable.h    Variable storage for the interpreter
│   ├── parser.h/.cpp    Recursive-descent parser: tokens → AST
│   └── main.cpp         Driver: runs the full pipeline
├── tests/
│   ├── test1.code       Valid program
│   ├── test2.code       Syntax error example
│   └── test3.code       Runtime error example
└── README.md
```

## Build and Run

```
g++ -std=c++17 src/*.cpp -o compiler
.\compiler tests\test1.code
```

Output of a run:
```
[Compiled to output.tac]
[Program output]
55
1
```

## Compiler Pipeline

```
Source (.code) → Lexer → Tokens → Parser → AST → Code Generator → output.tac
                                               └→ Interpreter   → program output
```

| Term | In this project |
|---|---|
| Source language | code |
| Implementation language | C++ |
| Target language | Three-address code |

## code Grammar

```
program     → statement*
statement   → let NAME = expr ;
            | NAME = expr ;
            | print expr ;
            | if ( expr ) block [ else block ]
            | while ( expr ) block
block       → { statement* }
expr        → additive [ (< | > | == | !=) additive ]
additive    → term ( (+ | -) term )*
term        → factor ( (* | /) factor )*
factor      → NUMBER | NAME | ( expr ) | - factor
```

Rules: integers only; names start with a letter; every simple statement ends with `;`;
`if`/`while` bodies always use `{ }`; conditions treat 0 as false and non-zero as true.

## Example Program

```
let n = 5;
let result = 1;
while (n > 0) {
    result = result * n;
    n = n - 1;
}
print result;
```

Generated three-address code:
```
n = 5
result = 1
L1:
t1 = n > 0
ifFalse t1 goto L2
t2 = result * n
result = t2
t3 = n - 1
n = t3
goto L1
L2:
print result
```

## Error Handling

| Input | Output |
|---|---|
| `let x = 5` (no `;`) | `Syntax Error (line 1): Expected ';' but found 'print'` |
| `print y;` (undefined) | `Runtime Error (line 1): Undefined variable 'y'` |
| `let x = 10 / 0;` | `Runtime Error (line 1): Division by zero` |
| `let x = 5 @ 3;` | `Lexer Error (line 1): Unexpected character '@'` |

## OOP Concepts Used

| Concept | Where |
|---|---|
| Encapsulation | `Token`, `SymbolTable`, `Lexer` keep data private behind getters/methods |
| Abstraction | `Expr` and `Stmt` are abstract classes with pure virtual `evaluate()` / `execute()` / `generate()` |
| Inheritance | All AST nodes derive from `Expr` or `Stmt`; all errors derive from `CompilerError` |
| Runtime polymorphism | `left->evaluate(table)` dispatches to the correct node type; `e.stage()` in the catch block |
| Exception handling | Each stage throws its own error type; `main` catches by base-class reference |
| RAII / smart pointers | `std::unique_ptr` owns every tree node — no manual `delete` |
| STL | `vector`, `map`, `string`, `unique_ptr` |

## Possible Extensions

Each of these is one new node class plus a parser rule: `%` operator, `&&`/`||`,
functions, strings, comments in source.