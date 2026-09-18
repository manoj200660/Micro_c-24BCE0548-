# Micro C Compiler

## Project Description

Micro C Compiler is a simplified compiler project based on a small C-like programming language.

The project is designed to demonstrate the major stages involved in compiler construction, starting from lexical analysis and progressing towards syntax analysis, semantic analysis, intermediate code generation and target code generatiion.

## Current Prototype

The current working prototype implements the lexical analysis phase.

The lexical analyzer reads a Micro C source program and converts the source characters into tokens such as keywords, identifiers, literals, operators and delimiters.

## Technologies

- Programming Language: C
- Standard: C11
- IDE: Visual Studio Code
- Compiler: GCC
- Target Architecture: x86-64
- Planned Target Code: x86-64 assembly

## Project Structure

- include/token.h - Token definitions
- include/lexer.h - Lexer declarations
- src/token.c - Token name handling
- src/lexer.c - Lexical analyzer implementation
- src/main.c - Main driver program
- tests/sample.mc - Sample Micro C program
- Makefile - Build instructions

## Build

```text
gcc -Wall -Wextra -std=c11 -Iinclude src/main.c src/lexer.c src/token.c -o microc.exe