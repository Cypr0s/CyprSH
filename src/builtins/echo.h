#ifndef ECHO_H
#define ECHO_H

#include "executor/execute_types.h"


StatusEnum builtinEcho(int16_t argc, char** argv, ExecuteEnvironmentPtr env);

#endif