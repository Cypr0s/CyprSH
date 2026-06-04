# Changelog

All notable changes to CyprSH will be documented here.

## [Unreleased]

### Added
- Lexer with full POSIX token recognition
- Recursive descent parser following POSIX grammar
- Environment variables handling
- Executor(external commands)
- Redirections
- Pipelines
- And/Or handling(`&&`/`||`)
- Interactive mode

### Planned
- Dollar single quotes (`$'...'`)
- IO_LOCATION token type
- Heredocs(`<<<`, `<<`, `<<-`)
- Word expansion
- Executor(Builtins, Compound commands, functions)
- Job control
- Signal handling
- Interactive/script/inline mode distinction

### Known Issues
- Background processes(`&`) currently create zombies until exit of CyprSH
