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
```

## Status
Work in progress. Currently implemented:
- [x] Lexer
- [x] Parser
- [X] Executor (partially, only external commands)
- [x] Pipelines
- [x] Redirections
- [ ] Word expansion
- [ ] Builtins
- [ ] Job control

## Examples
```sh
cyprSH> echo "Hello world"
Hello world

cyprSH> ls -la | wc -l
14

cyprSH> echo first > test.txt
cyprSH> echo second >> test.txt
cyprSH> cat test.txt
first
second

cyprSH> date; whoami; pwd
Thu Jun  4 07:14:15 PM CEST 2026
luptakk
/home/luptakk

cyprSH> echo a && echo b
a
b

cyprSH> false || echo "fallback"
fallback
```

## License
GPL v3