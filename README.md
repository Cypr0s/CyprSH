# CyprSH

A minimalistic POSIX-inspired shell written in C from scratch to deepen my understanding of systems programming.

## Features

- Lexer with POSIX shell token recognition
- Recursive descent parser
- Abstract syntax tree (AST)
- External command execution via fork + execve
- Pipelines (cmd1 | cmd2 | cmd3)
- I/O redirections (>, >>, <, <>, >|, >&, <&)
- Command lists (;, &&, ||)
- Compound commands (if, while, until, for, case)
- Background execution (&)
- Environment variable assignments (VAR=value cmd)
- Quoting (single, double, backslash)
- Word expansion
- Builtins: cd, exit, export, unset, pwd, echo, true, false, :
- Shell functions
- Interactive mode with readline history

## Limitations

CyprSH does not aim to implement the complete POSIX shell specification for now. The following features outside the scope of the project:

- Heredocs

## Building
```sh

make # release build

make debug # debug build with sanitizers

```

**## Usage**

```sh

./cyprsh-linux-x86_64 # interactive mode

./cyprsh-linux-x86_64 -c 'echo "Hello world"' # command mode

./cyprsh-linux-x86_64 script.sh # script mode

```


## Examples

```sh

cyprSH> echo "Hello world"

Hello world

cyprSH> echo $HOME

/home/luptakk

cyprSH> echo ~

/home/luptakk

cyprSH> ls -la | wc -l

14

cyprSH> echo first > test.txt

cyprSH> echo second >> test.txt

cyprSH> cat test.txt

first

second

cyprSH> date; whoami; pwd

Thu Jun 4 07:14:15 PM CEST 2026

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

cyprSH> greet() { echo "Hello $1"; }

cyprSH> greet world

Hello world

cyprSH> greet() { echo "Hi $1"; }

cyprSH> greet world

Hi world

cyprSH> foo() { echo "foo"; }

cyprSH> bar() { echo "bar"; }

cyprSH> foo

foo

cyprSH> bar

bar

```

## Status

- [x] Lexer

- [x] Parser

- [x] Abstract syntax tree

- [x] Executor (external commands)

- [x] Pipelines

- [x] Redirections

- [x] Command lists

- [x] Compound commands

- [x] Background execution

- [x] Environment variable assignments

- [x] Quoting

- [x] Essential builtins

- [x] Word expansion

- [x] Functions

- [ ] Arithmetic expansion ($((...))), Command substitution ($(...))

- [ ] Job control

- [ ] Signal handling

## License

GPL v3
