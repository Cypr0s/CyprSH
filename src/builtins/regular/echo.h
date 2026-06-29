/**
 * @file        echo.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Echo builtin command
 */

#ifndef ECHO_H
#define ECHO_H

#include "executor/execute_types.h"

/** @brief Print arguments separated by spaces
 *  @param argc Argument count
 *  @param argv Arguments to print
 *  @param env Execute environment
 *  @return Status code
 */
StatusEnum builtinEcho(int16_t argc, char** argv, ExecuteEnvironmentPtr env);

#endif