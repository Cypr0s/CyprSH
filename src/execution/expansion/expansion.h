/**
 * @file        expansion.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Parameter and variable expansion
 */

#ifndef EXPANSION_H
#define EXPANSION_H

#include "error.h"
#include "data-structures/expander.h"
#include "execution/expansion/parameter.h"
#include "execution/expansion/tilde.h"
#include "execution/execute-type.h"


/** @brief Expand word with parameter substitution
 *  @param env Execution environment
 *  @param input Input word string
 *  @param input_types Array of character type flags
 *  @param output Output pointer for expanded string
 *  @return Status code
 */
StatusEnum expandWord(ExecuteEnvironmentPtr env, const char* input, const int8_t* input_types, char** output);

#endif // EXPANSION_H