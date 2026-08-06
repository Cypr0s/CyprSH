/**
 * @file        false.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   False (failure) builtin command
 */

#ifndef FALSE_H
#define FALSE_H

#include "execution/execute-type.h"

/** @brief Return failure status
 *  @param argc Argument count (unused)
 *  @param argv Argument values (unused)
 *  @param env Execute environment
 *  @return Status code
 */
StatusEnum builtinFalse(int16_t argc __attribute__((unused)), 
                        char** argv __attribute__((unused)), 
                        ExecuteEnvironmentPtr env
                    );

#endif