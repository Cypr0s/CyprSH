#include "builtins/export.h"

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