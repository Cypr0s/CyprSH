#ifndef TRUE_H
#define TRUE_H

#include "executor/execute.h"

StatusEnum builtinTrue(int16_t argc __attribute__((unused)), 
                        char** argv __attribute__((unused)), 
                        ExecuteEnvironmentPtr env __attribute__((unused))
                    );

#endif