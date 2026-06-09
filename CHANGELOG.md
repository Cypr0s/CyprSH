# Changelog

All notable changes to CyprSH will be documented here.

## [1.0.0] - 2026-06-04

### Added
- Lexer with full POSIX token recognition
- Recursive descent parser following POSIX grammar
- Environment variables handling
- Executor(external commands)
- Redirections
- Pipelines
- And/Or handling(`&&`/`||`)
- Interactive mode
- Builtins: `cd`, `exit`, `export`, `unset`, `pwd`, `echo`, `true`, `false`, `:`
- Builtins working in pipelines

### Planned
- Dollar single quotes (`$'...'`)
- IO_LOCATION token type
- Heredocs(`<<<`, `<<`, `<<-`)
- Word expansion
- Executor(Compound commands, functions)
- Job control
- Signal handling
- Interactive/script/inline mode distinction
- All builtin commands

### Known Issues
- Background processes(`&`) currently create zombies until exit of CyprSH