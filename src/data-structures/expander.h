/**
 * @file        expander.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-10-6
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Word expansion state and structures
 */

#ifndef EXPANDER_H
#define EXPANDER_H

#include "error.h"
#include "data-structures/buffer-type.h"
#include "execution/execute-type.h"
#include "data-structures/stack.h"
#include "data-structures/char-buffer.h"

#define DEFAULT_OUTPUT_SIZE 32
#define DEFAULT_NAME_SIZE 8

#define MAX_PID_BYTES 16
#define MAX_EXEC_STATUS_BYTES 7

typedef enum {
    EXP_NORMAL, // unqoted chars
    EXP_TILDE, // ~
    EXP_DOLLAR, // $
    EXP_BRACE, // ${}
    EXP_ARITHMETIC, // $(())
    EXP_COMMAND_SUB, // $()
} ExpanderStateEnum;

typedef struct {
    const char* input;
    const int8_t* input_types;
    size_t input_length;
    size_t current_input_pos;
    ExecuteEnvironmentPtr env;
    Stack state_stack;
    CharBuffer output;
    CharBuffer name;
} Expander, *ExpanderPtr;

StatusEnum expanderCtor(ExpanderPtr exp, ExecuteEnvironmentPtr env, const char* input, const int8_t* input_types);

void expanderDtor(ExpanderPtr exp);

#endif // EXPANDER_H