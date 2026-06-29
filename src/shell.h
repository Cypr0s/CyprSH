/**
 * @file        shell.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Interactive, script, and string execution interface
 */

#ifndef SHELL_H
#define SHELL_H

#include "utils/file.h"
#include "utils/env.h"
#include <readline/readline.h>
#include <readline/history.h>
#include "parser/syntax.h"
#include "lexer/lexer.h"
#include "executor/execute.h"

#define HISTORY_FILE_NAME "CyprSH_history"

/** @brief Runs shell in interactive mode with readline history
 *  @param env_table Pointer to environment variable table
 *  @return Status code
 */
StatusEnum runInteractive(HashTablePtr env_table);

/** @brief Executes shell commands from file input
 *  @param input File stream to read commands from
 *  @param env_table Pointer to environment variable table
 *  @return Status code
 */
StatusEnum runScript(FILE* input, HashTablePtr env_table);

/** @brief Executes a single command string
 *  @param string_input Command string to execute
 *  @param env_table Pointer to environment variable table
 *  @return Status code
 */
StatusEnum runString(char* string_input, HashTablePtr env_table);

/** @brief Prints AST tree structure for debugging
 *  @param node AST node to print
 *  @param depth Current recursion depth
 */
void printAST(ASTNodePtr node, int depth);

#endif // SHELL_H