#include "builtins/pwd.h"

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