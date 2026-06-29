/**
 * @file        echo.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Echo builtin implementation
 */

#include "builtins/echo.h"

/** @brief Print arguments separated by spaces
 *  @param argc Argument count
 *  @param argv Arguments to print
 *  @param env Execute environment
 *  @return Status code
 */
StatusEnum builtinEcho(int16_t argc, char** argv, ExecuteEnvironmentPtr env) {
    for(int16_t i = 1; i < argc; i++) {
        printf("%s", argv[i]);
        if(i < argc - 1) {
            printf(" ");
        }
    }

    printf("\n");
    
    env->last_exec_status = 0;
    return SUCCESS;
} 