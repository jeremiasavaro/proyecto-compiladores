# Compilers Project
The project consists of the design and implementation of a compiler for a simple programming language
called TDS25.

## Compiler phases
- Lexical Analyzer:
  The Lexical Analyzer takes a file with TDS25 source code as input and returns tokens.
- Syntactic Analyzer:
  The Syntactic Analyzer takes the sequence of tokens as input and verifies that this sequence is
  a valid sequence, that is, that it complies with the syntactic specification of the language.
- Semantic Analyzer:
  This stage verifies the semantic rules of the language, for example, type compatibility, visibility,
  and scope of identifiers, etc.
- Intermediate Code Generator:
  This stage of the compiler returns an intermediate representation (IR) of the code. From this intermediate representation,
  the object code will be generated.
- Object Code Generator:
  In this stage, x86-64 assembly code (without optimizations) is generated from the intermediate code.
- Optimizer

## Branches for tasks
- Lexical and syntactic analyzer: syntactic-analyzer
- Semantic analyzer: semantic-analyzer

## Dependencies
- flex
- bison
- gcc
- make

## Compilation
```sh
  make
```
Generates the binary called `parser`.

## Use
From file:
```sh
  ./parser input.txt
```
Or by stdin (Ctrl+D to exit):
```sh
  ./parser
```

### Members:
- Avaro Jeremias Jose
- Cornejo Mateo Andres
- Marquez Regis Máximo Pablo
