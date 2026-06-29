/**
 * @file        unset.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Unset builtin special command
 */

#ifndef BUILTIN_UNSET_H
#define BUILTIN_UNSET_H

#include "executor/execute_types.h"

/** @brief Unset environment variables
 *  @param argc Argument count
 *  @param argv Variable names to unset
 *  @param env Execute environment
 *  @return Status code
 */
StatusEnum builtinUnset(int16_t argc, char** argv, ExecuteEnvironmentPtr env);

#endif