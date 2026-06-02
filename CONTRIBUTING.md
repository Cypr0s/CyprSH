# Contributing to CyprSH

Hey! I'm a CS student building this shell from scratch as a learning project.
If you found this interesting enough to want to contribute, that makes me really happy.

## Building

### Dependencies
- gcc
- make
- libreadline-dev

### Build
```sh
# debug build (with sanitizers)
make debug

# release build
make release

# clean
make fclean
```

## Project Structure - subject to change
```
src/
├── lexer/          # tokenizer
├── parser/         # recursive descent parser
├── executor/       # command execution
├── data_structures/ # AST, stack, hash table
├── utils/          # env, file, strings
├── error.h         # error codes
└── shell.c         # main shell loop
```

## Code Style
- C11 or newer
- PascalCase for structs and types: `LexerPtr`, `TokenTypeEnum`
- camelCase for functions: `getToken`, `analyzeList`
- snake_case for variables: `current_token`, `buffer_pos`
- ALL_CAPS for macros and enum values: `TOKEN_WORD`, `FLAG_BACKGROUND`
- `Ptr`/ `PtrPtr` suffix for pointer typedefs: `LexerPtr`, `ASTNodePtr`
- No warnings with `-Wall -Wextra`

## Reporting Bugs
Open an issue with:
- Shell command that caused the bug
- Expected behavior
- Actual behavior
- Output of `make debug && ./cyprsh`

## Current Status
The project is under active development. See the README
for what is and isn't implemented yet.

Also project will not be handling every possible edge case so be prepared for that!