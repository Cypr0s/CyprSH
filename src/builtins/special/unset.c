#include "builtins/unset.h"

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