# CyprSH

A minimalistic POSIX-compliant shell written in C from scratch to deepen my understanding of systems programming.

## Features
- Lexer with full POSIX token recognition
- Recursive descent parser
- Interactive mode with readline history
- Script mode

## Building
```sh
make          # release build
make debug    # debug build with sanitizers
```

## Usage
```sh
./cyprsh              # interactive mode
./cyprsh script.sh    # script mode
```

## Status
Work in progress. Currently implemented:
- [x] Lexer
- [x] Parser
- [ ] Executor
- [ ] Word expansion
- [ ] Builtins
- [ ] Job control

## License
GPL v3