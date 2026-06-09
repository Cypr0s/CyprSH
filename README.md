# CyprSH

A minimalistic POSIX-compliant shell written in C from scratch to deepen my understanding of systems programming.

## Features
- Lexer with full POSIX token recognition
- Recursive descent parser
- External command execution via fork + execve
- Pipelines (`cmd1 | cmd2 | cmd3`)
- I/O redirections (`>`, `>>`, `<`, `<>`, `>|`, `>&`, `<&`)
- Command lists (`;`, `&&`, `||`)
- Background execution (`&`)
- Environment variable assignments (`VAR=value cmd`)
- Quoting (single, double, backslash)
- Builtins: `cd`, `exit`, `export`, `unset`, `pwd`, `echo`, `true`, `false`, `:`
- Interactive mode with readline history

## Building
```sh
make          # release build
make debug    # debug build with sanitizers
```

## Usage
```sh
./cyprsh              # interactive mode
```

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

cyprSH> cd /tmp && pwd
/tmp

cyprSH> export FOO=bar
cyprSH> export | head -3
export XDG_SESSION_CLASS="user"
export __ETC_PROFILE_NIX_SOURCED="1"
export _="./cyprsh"
```

## Status
- [x] Lexer
- [x] Parser
- [x] Executor (external commands)
- [x] Pipelines
- [x] Redirections
- [x] Essential builtins
- [ ] Word expansion (`$VAR`, `${VAR}`, globbing)
- [ ] Compound command execution (`if`/`while`/`for`/`case`)
- [ ] Functions
- [ ] Heredocs
- [ ] Job control
- [ ] Signal handling

## License
GPL v3