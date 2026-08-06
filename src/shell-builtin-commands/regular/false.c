/**
 * @file        false.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   False (failure) builtin implementation
 */

#include "shell-builtin-commands/regular/false.h"

/** @brief Return failure status
 *  @param argc Argument count (unused)
 *  @param argv Argument values (unused)
 *  @param env Execute environment
 *  @return Status code
 */
StatusEnum builtinFalse(int16_t argc __attribute__((unused)),
                         char** argv __attribute__((unused)), 
                         ExecuteEnvironmentPtr env
                    ) {
    env->last_exec_status = 1;
    return SUCCESS;
}