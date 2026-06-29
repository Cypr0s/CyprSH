/**
 * @file        unset.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Unset builtin special implementation
 */

#include "builtins/unset.h"

/** @brief Unset environment variables
 *  @param argc Argument count
 *  @param argv Variable names to unset
 *  @param env Execute environment
 *  @return Status code
 */
StatusEnum builtinUnset(int16_t argc, char** argv, ExecuteEnvironmentPtr env) {
    if(argc < 2) {
        env->last_exec_status = 0;
        return SUCCESS;
    }

    // remove all (we dont care if they dont exits)
    for(int16_t i = 1; i < argc; i++) {
        hashTableRemove(env->env_table, argv[i]);
    }

    env->last_exec_status = 0;
    return SUCCESS;
}