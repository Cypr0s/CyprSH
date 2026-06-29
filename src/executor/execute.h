/**
 * @file        execute.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   AST node execution interface
 */

#ifndef EXECUTE_H
#define EXECUTE_H

#include "executor/execute_types.h"
#include "utils/env.h"
#include "utils/strings.h"
#include "utils/file.h"
#include "builtins/special/special_builtins.h"
#include "builtins/regular/regular_builtins.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

/** @brief Execute AST node in given environment
 *  @param node AST node to execute
 *  @param env Execution environment with variables and state
 *  @return Status code
 */
StatusEnum executeNode(ASTNodePtr node, ExecuteEnvironmentPtr env);

/** @brief Initialize executor environment
 *  @param env Environment to initialize
 *  @param p_env Pointer to process environment table
 */
void executorCtor(ExecuteEnvironmentPtr env, HashTablePtr p_env);

#endif // EXECUTE_H