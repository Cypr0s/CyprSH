#include "builtins/true.h"


StatusEnum builtinTrue(int16_t argc __attribute__((unused)),
                         char** argv __attribute__((unused)), 
                         ExecuteEnvironmentPtr env
                    ) {
    env->last_exec_status = 0;
    return SUCCESS;
}