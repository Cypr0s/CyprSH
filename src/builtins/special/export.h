/**
 * @file        export.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Export builtin special command
 */

#ifndef BUILTIN_EXPORT_H
#define BUILTIN_EXPORT_H

#include "executor/execute_types.h"

/** @brief Export/list environment variables
 *  @param argc Argument count
 *  @param argv Argument values (key=value pairs)
 *  @param env Execute environment
 *  @return Status code
 */
StatusEnum builtinExport(int16_t argc, char** argv, ExecuteEnvironmentPtr env);

#endif