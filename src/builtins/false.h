#ifndef FALSE_H
#define FALSE_H

#include "executor/execute.h"

StatusEnum builtinFalse(int16_t argc __attribute__((unused)), 
                        char** argv __attribute__((unused)), 
                        ExecuteEnvironmentPtr env __attribute__((unused))
                    );

#endif