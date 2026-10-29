# Micro C Compiler

## Design and Development of a Compiler for a Simplified C-Like Language

A compiler design laboratory project implemented in C using a handwritten lexer, parser, semantic analysis module, Three-Address Code (TAC) generator, and target-code generation module.

## 1. Project Overview

The Micro C Compiler is designed to demonstrate the main stages involved in compiling a simplified C-like programming language.

The project accepts a source program written in the Micro C language and processes it through multiple compiler stages. It identifies tokens, checks syntax, performs basic semantic checks, generates intermediate code in the form of Three-Address Code, and translates the generated TAC into x86-64 assembly code.

The project is developed in C using the C11 standard and can be built using GCC.

## 2. Objectives

- To understand the basic working of a compiler.
- To implement lexical analysis for identifying tokens.
- To perform syntax analysis on the source program.
- To implement basic semantic checks.
- To generate Three-Address Code as an intermediate representation.
- To generate x86-64 assembly code from the intermediate code.
- To test the compiler using valid and invalid source programs.

## 3. Compiler Architecture

The project follows this compilation pipeline:

```text
Micro C Source Program
          |
          v
   Lexical Analysis
          |
          v
   Syntax Analysis
          |
          v
   Semantic Analysis
          |
          v
Three-Address Code (TAC)
          |
          v
 Target Code Generation
          |
          v
   x86-64 Assembly
```

### 3.1 Lexical Analysis

The lexical analyzer reads the source program and identifies tokens such as keywords, identifiers, operators, literals, and punctuation symbols.

### 3.2 Syntax Analysis

The parser checks whether the source program follows the grammar rules supported by the Micro C language.

### 3.3 Semantic Analysis

The semantic analysis module performs basic checks, including checking declarations and detecting certain duplicate or undeclared identifiers.

The semantic checks are limited to the functionality implemented in the current version.

### 3.4 Three-Address Code Generation

The TAC generator translates supported source statements into an intermediate representation. It uses temporary variables to represent intermediate expression results and labels to represent control flow.

For example:

```text
t1 = y * 2
t2 = x + t1
result = t2
```

### 3.5 Target Code Generation

The target-code generation module translates supported TAC instructions into x86-64 assembly using AT&T syntax.

The generated assembly includes instructions for supported arithmetic operations, comparisons, conditional branches, labels, array accesses, printing, and returning a result.

The generated assembly targets Linux x86-64 and may require a compatible Linux environment to assemble and execute.

## 4. Technologies Used

- **Programming language:** C
- **Language standard:** C11
- **Compiler:** GCC
- **Development environment:** Visual Studio Code
- **Target architecture:** x86-64
- **Target assembly syntax:** AT&T
- **Version control:** Git and GitHub

## 5. Project Structure

```text
Micro C Compiler/
├── .vscode/
├── include/
│   ├── ast.h
│   ├── lexer.h
│   ├── parser.h
│   ├── semantic.h
│   ├── symbol_table.h
│   ├── tac.h
│   ├── token.h
│   └── codegen.h
├── src/
│   ├── ast.c
│   ├── lexer.c
│   ├── main.c
│   ├── parser.c
│   ├── semantic.c
│   ├── symbol_table.c
│   ├── tac.c
│   ├── token.c
│   └── codegen.c
├── tests/
│   ├── sample.mc
│   ├── invalid_syntax.mc
│   └── invalid_semantic.mc
├── Makefile
└── README.md
```

The exact files present may vary depending on the latest project changes.

## 6. Requirements

Install the following software before building the project:

- GCC C compiler
- GNU Make, if using the Makefile
- Visual Studio Code or another C development environment
- WSL or another compatible Linux environment for assembling and running the generated Linux x86-64 assembly

## 7. Building the Project

### Option A: Using the Makefile

Open a terminal in the project directory and run:

```bash
make
```

This compiles the source files and creates the compiler executable.

To run the compiler with the sample program:

```bash
make run
```

To remove generated build and assembly files:

```bash
make clean
```

### Option B: Using GCC directly on Windows

From PowerShell, run:

```powershell
cd "D:\Micro C Compiler"

gcc -Wall -Wextra -std=c11 -Iinclude src/main.c src/lexer.c src/token.c src/parser.c src/semantic.c src/tac.c src/codegen.c -o microc.exe
```

Run the compiler using the sample source program:

```powershell
.\microc.exe tests\sample.mc
```

The compiler should display its implemented analysis and code-generation stages. The generated assembly is saved to `output.s` when target-code generation succeeds.

## 8. Sample Program

The file `tests/sample.mc` demonstrates supported language features.

```c
func main_prog() {
    int x;
    int y;
    int result;
    int i;
    int arr[5];

    x = 10;
    y = 3;

    result = x + y * 2;
    arr[0] = result;
    arr[1] = x - y;

    if (x > y) {
        result = arr[0] + arr[1];
    } else {
        result = x * y;
    }

    for (i = 0; i < 5; i = i + 1) {
        result = result + i;
    }

    print result;
    return result;
}
```

This example contains variable declarations, arithmetic expressions, array accesses, an if-else statement, a for loop, printing, and a return statement.

## 9. Testing

The project includes test programs for checking the implemented compiler stages.

| Test file | Purpose |
|---|---|
| `tests/sample.mc` | Demonstrates the supported language features and compilation pipeline |
| `tests/invalid_syntax.mc` | Tests syntax-error handling |
| `tests/invalid_semantic.mc` | Tests selected semantic-error checks |

The sample program is expected to print `33` when the generated target program correctly implements the sample's operations.

Run the compiler on the sample first, then test the invalid programs separately. Record the actual output from your current build in the final project report.

## 10. Running the Generated Assembly

The generated assembly targets Linux x86-64. Generating `output.s` on Windows does not, by itself, prove that the assembly executes correctly.

Using WSL with a compatible Linux GCC toolchain, open the project directory and run:

```bash
cd "/mnt/d/Micro C Compiler"

gcc -no-pie output.s -o output

./output

echo $?
```

The expected printed value for the sample program is `33`. If the program returns that value from `main`, the shell exit status will typically be `33`.

Successful execution should be verified in the target environment before reporting it as a tested result.

## 11. Current Scope and Limitations

This project implements a simplified C-like language rather than the complete C programming language.

- Only the syntax and language constructs supported by the current implementation can be compiled.
- Semantic analysis performs basic checks and is not a complete C type or scope checker.
- Target-code generation supports the TAC instruction patterns implemented in the backend.
- The generated assembly targets Linux x86-64 and is not directly a native Windows executable.
- Advanced C features and unsupported instructions are outside the current scope.

## 12. Future Enhancements

- Expand syntax and semantic validation.
- Improve symbol-table and scope handling.
- Add more data types and language constructs.
- Extend target-code generation to support additional instructions.
- Improve error reporting and recovery.
- Add automated regression tests for every compiler stage.
- Add optimization passes for intermediate code.

## 13. Conclusion

The Micro C Compiler project demonstrates the implementation of a simplified compilation pipeline, from source-code tokenization through syntax and basic semantic analysis, intermediate-code generation, and target assembly generation.

It provides practical experience with compiler design concepts and a foundation for future improvements to language support, error handling, testing, and code generation.

## 14. Repository

GitHub repository:

https://github.com/manoj200660/Micro_c-24BCE0548-
