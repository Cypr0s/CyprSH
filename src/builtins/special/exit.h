/**
 * @file        exit.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Exit builtin special command
 */

#ifndef BUILTIN_EXIT_H
#define BUILTIN_EXIT_H

#include "executor/execute_types.h"

/** @brief Exit shell with optional exit code
 *  @param argc Argument count
 *  @param argv Argument values (argv[1] is exit code)
 *  @param env Execute environment
 *  @return Status code
 */
StatusEnum builtinExit(int16_t argc, char** argv, ExecuteEnvironmentPtr env);

#endif