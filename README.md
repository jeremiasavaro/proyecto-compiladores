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
- Optimizations: In this stage, optimizations to the generated code are done. Some of the optimizations done are: propagation of constants, elimination of dead code, division by powers of 2 using shifts, reuse of temporals in intermediate code, etc.

## Branches for tasks
- Lexical and syntactic analyzer: syntactic-analyzer
- Semantic analyzer: semantic-analyzer
- Intermediate code generator: intermediate-code-generate
- Object code generator: object-code-generator
- Optimizations: optimization

## Dependencies
- flex
- bison
- gcc
- make

- ## Additional Dependencies
- bash (used by `link.sh` and test scripts)
- clang (optional alternative to `gcc`)
- POSIX environment (macOS / Linux x86\-64)

## Project Structure
- `lex.l` / `parser.y`  Flex / Bison specifications
- `main.c`  CLI and pipeline orchestration
- `tree/`  AST node definitions
- `symbol_table/`  Scoped symbol table implementation
- `semantic_analyzer/`  Semantic checks
- `intermediate_code/`  IR generation and dumps
- `optimization/`  IR and temporary reuse optimizations
- `object_code/`  x86\-64 assembly emission
- `error_handling/`  Centralized error reporting
- `print_funcs.h` and `print_utilities/`  Debug / dump helpers
- `utils/`  Support functions
- `tests/`  Correct and incorrect `.ctds` programs for testing (run `./tests/test.sh`)
- `link.sh`  Assembles and links emitted assembly into executable

- ## Compilation
```sh
  make && eval "$(make -s env)"
```
Generates the binary called `ctds` and make an alias so you can use `ctds <file>`.

## Cleaning
```sh
make clean
```

## Use
```sh
   ctds input.ctds
```

## Command Line Options
Run `ctds -h` to see help.
- `-o <file>`  Output base name (default: `out`)
- `-t | -target <stage>`  `scan | parse | codinter | assembly`
- `-opt`  Enable optimizations
- `-d | -debug`  Dump internal structures (tokens, AST, IR, temps)
- `-h | -help`  Show usage

## Pipeline Stages
- `scan`  Tokenization only
- `parse`  AST build and semantic analysis
- `codinter`  Intermediate code generation
- `assembly`  Emit `.s`
- Default (no `-t`) runs full pipeline, links and generates an executable

### Members:
- Avaro Jeremias Jose
- Cornejo Mateo Andres
- Marquez Regis Máximo Pablo
