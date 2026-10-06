# Changelog

All notable changes to CyprSH will be documented here.

## [1.1.0] - 2026-10-06

### Added
- Compound commands (if, while, until, for, case)

- Shell functions

- Function redefinition and persistent function definitions

- Parameter expansion ($VAR, ${VAR})

- Special parameters ($?, $$, $!, $#, $@, $*)

- Positional parameter expansion ($0–$9)

- Parameter expansion operators (:-, -, :=, =, :?, ?, :+, +)

- Parameter length expansion (${#VAR})

- Tilde expansion (~, ~/path, ~user, ~user/path)

- Environment variable assignments

- Single, double and backslash quoting

### Known Limitations

- Background processes (&) still create zombies until exit of CyprSH

## [1.0.1] - 2026-06-10

### Fixed

- History file now persists across sessions (bugged due to builtins)

## [1.0.0] - 2026-06-04

### Added

- Lexer with full POSIX token recognition

- Recursive descent parser following POSIX grammar

- Environment variables handling

- Executor (external commands)

- Redirections

- Pipelines

- And/Or handling (&&/||)

- Interactive mode

- Builtins: cd, exit, export, unset, pwd, echo, true, false, :

- Builtins working in pipelines

### Planned

- Dollar single quotes ($'...')

- IO_LOCATION token type

- Heredocs (<<<, <<, <<-)

- Word expansion

- Executor (Compound commands, functions)

- Job control

- Signal handling

- Interactive/script/inline mode distinction

- All builtin commands

### Known Issues

- Background processes (&) currently create zombies until exit of CyprSH