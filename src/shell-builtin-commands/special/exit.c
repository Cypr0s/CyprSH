/**
 * @file        exit.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Exit builtin special implementation
 */

#include "shell-builtin-commands/special/exit.h"

/** @brief Exit shell with optional exit code
 *  @param argc Argument count
 *  @param argv Argument values (argv[1] is exit code)
 *  @param env Execute environment
 *  @return Status code
 */
StatusEnum builtinExit(int16_t argc, char** argv, ExecuteEnvironmentPtr env) {
    if(argc > 2) {
        printError("exit", "too many arguments");
        env->last_exec_status = 1;
        return SUCCESS;
    }
    
    // exit with different status
    if(argc == 2) {
        char* endptr;
        long val = strtol(argv[1], &endptr, 10);
        if(*endptr != '\0') {
            printError("exit", "%s: numeric argument required", argv[1]);
            env->last_exec_status = 2; // posix misuse
        } else {
            env->last_exec_status = (uint8_t)(val & 255);  // mod 256
        }
    }
    // else use existing last_exec_status

    env->flags |= EXEC_FLAG_EXIT;
    return SUCCESS;
}