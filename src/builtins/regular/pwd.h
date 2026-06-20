#ifndef PWD_H
#define PWD_H

#include "executor/execute_types.h"
#include <unistd.h>

StatusEnum builtinPwd(int16_t argc __attribute__((unused)), 
                    char** argv __attribute__((unused)), 
                    ExecuteEnvironmentPtr env __attribute__((unused))
                );

#endif