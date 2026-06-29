/**
 * @file        pwd.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Print working directory builtin implementation
 */

#include "builtins/pwd.h"

/** @brief Print current working directory
 *  @param argc Argument count (unused)
 *  @param argv Argument values (unused)
 *  @param env Execute environment
 *  @return Status code
 */
StatusEnum builtinPwd(int16_t argc __attribute__((unused)), 
                    char** argv __attribute__((unused)), 
                    ExecuteEnvironmentPtr env
                ) {
    char cwd[PATH_MAX];

    if(getcwd(cwd, sizeof(cwd)) == NULL) {
        printError("pwd", "%s", strerror(errno));
        env->last_exec_status = 1;
        return SUCCESS;
    }
    
    printf("%s\n", cwd);
    env->last_exec_status = 0;
    return SUCCESS;
}