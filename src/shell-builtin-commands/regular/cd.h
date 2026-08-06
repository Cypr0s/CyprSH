/**
 * @file        cd.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Change directory builtin command
 */

#ifndef CD_H
#define CD_H

#include "execution/execute-type.h"
#include <unistd.h>

/** @brief Change working directory
 *  @param argc Argument count
 *  @param argv Argument values (argv[1] is target directory)
 *  @param env Execute environment with PWD/OLDPWD
 *  @return Status code
 */
StatusEnum builtinCd(int16_t argc, char** argv, ExecuteEnvironmentPtr env);

#endif