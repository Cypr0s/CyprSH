#include "builtins/echo.h"

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