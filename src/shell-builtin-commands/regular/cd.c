/**
 * @file        cd.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Change directory builtin implementation
 */

#include "shell-builtin-commands/regular/cd.h"

/** @brief Change working directory
 *  @param argc Argument count
 *  @param argv Argument values (argv[1] is target directory)
 *  @param env Execute environment with PWD/OLDPWD
 *  @return Status code
 */
StatusEnum builtinCd(int16_t argc, char** argv, ExecuteEnvironmentPtr env) {
    char* target = NULL;
    int8_t print_target = 0;

    // no arg -> $HOME
    if(argc < 2) {
        hashTableGetValue(env->env_table, "HOME", &target);
        if(target == NULL) {
            printError("cd", "HOME not set");
            env->last_exec_status = 1;
            return SUCCESS;
        }
    // `-` -> $OLDPWD 
    } else if(streq(argv[1], "-")) {
        hashTableGetValue(env->env_table, "OLDPWD", &target);
        if(target == NULL) {
            printError("cd", "OLDPWD not set");
            env->last_exec_status = 1;
            return SUCCESS;
        }
        print_target = 1;
    } else {
        target = argv[1];
    }

    // save current PWD as OLDPWD
    char* old_pwd = NULL;
    hashTableGetValue(env->env_table, "PWD", &old_pwd);
    if(old_pwd != NULL) {
        hashTableInsert(env->env_table, "OLDPWD", old_pwd);
    }

    // perform the change
    if(chdir(target) == -1) {
        printError("cd", "%s: %s", target, strerror(errno));
        env->last_exec_status = 1;
        return SUCCESS;
    }

    // print target if cd -
    if(print_target) {
        printf("%s\n", target);
    }

    // update PWD with the new absolute path
    char new_pwd[PATH_MAX];
    if(getcwd(new_pwd, sizeof(new_pwd)) != NULL) {
        hashTableInsert(env->env_table, "PWD", new_pwd);
    }

    env->last_exec_status = 0;
    return SUCCESS;
}