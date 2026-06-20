#include "builtins/false.h"

StatusEnum builtinFalse(int16_t argc __attribute__((unused)),
                         char** argv __attribute__((unused)), 
                         ExecuteEnvironmentPtr env
                    ) {
    env->last_exec_status = 1;
    return SUCCESS;
}