#ifndef CD_H
#define CD_H

#include "executor/execute_types.h"
#include <unistd.h>


StatusEnum builtinCd(int16_t argc, char** argv, ExecuteEnvironmentPtr env);

#endif