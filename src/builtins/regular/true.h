/**
 * @file        true.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   True (success) builtin command
 */

#ifndef TRUE_H
#define TRUE_H

#include "executor/execute_types.h"

/** @brief Return success status
 *  @param argc Argument count (unused)
 *  @param argv Argument values (unused)
 *  @param env Execute environment
 *  @return Status code
 */
StatusEnum builtinTrue(int16_t argc __attribute__((unused)), 
                        char** argv __attribute__((unused)), 
                        ExecuteEnvironmentPtr env
                    );

#endif