/**
 * @file        export.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Export builtin special implementation
 */

#include "shell-builtin-commands/special/export.h"

/** @brief Export/list environment variables
 *  @param argc Argument count
 *  @param argv Argument values (key=value pairs)
 *  @param env Execute environment
 *  @return Status code
 */
StatusEnum builtinExport(int16_t argc, char** argv, ExecuteEnvironmentPtr env) {
    // no args list all env vars
    if(argc < 2) {
        HashTableIter iter;
        hashTableIterCtor(&iter, env->env_table);
        char* key;
        char* value;
        while(hashTableIterNext(&iter, &key, &value)) {
            printf("export %s=\"%s\"\n", key, value);
        }
        env->last_exec_status = 0;
        return SUCCESS;
    }

    // set all exports
    for(int16_t i = 1; i < argc; i++) {
        char* key;
        char* value;
        StatusEnum st = splitAssignment(argv[i], &key, &value);
        if(st != SUCCESS) {
            continue;
        }

        st = hashTableInsert(env->env_table, key, value);
        free(key);
        free(value);
        if(st != SUCCESS) {
            printError("export", "failed to set '%s'", argv[i]);
            env->last_exec_status = 1;
            return SUCCESS;
        }
    }

    env->last_exec_status = 0;
    return SUCCESS;
}