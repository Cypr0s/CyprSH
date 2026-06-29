/**
 * @file        pwd.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Print working directory builtin command
 */

#ifndef PWD_H
#define PWD_H

#include "executor/execute_types.h"
#include <unistd.h>

/** @brief Print current working directory
 *  @param argc Argument count (unused)
 *  @param argv Argument values (unused)
 *  @param env Execute environment
 *  @return Status code
 */
StatusEnum builtinPwd(int16_t argc __attribute__((unused)), 
                    char** argv __attribute__((unused)), 
                    ExecuteEnvironmentPtr env __attribute__((unused))
                );

#endif