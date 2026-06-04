#ifndef SHELL_H
#define SHELL_H

#include "utils/file.h"
#include "utils/env.h"
#include <readline/readline.h>
#include <readline/history.h>
#include "parser/syntax.h"
#include "lexer/lexer.h"
#include "executor/execute.h"

#define HISTORY_FILE_PATH "./CyprSH_history"

StatusEnum runShell(int32_t file_descriptor, HashTablePtr env_table);

void printAST(ASTNodePtr node, int depth);

#endif // SHELL_H