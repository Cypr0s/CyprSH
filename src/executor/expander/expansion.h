/**
 * @file        expansion.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Parameter and variable expansion
 */

#include "error.h"
#include "execute_types.h"
#include "data_structures/stack.h"
#include "data_structures/char_buffer.h"
#include "lexer/lexer.h"
#include <pwd.h>

#define DEFAULT_OUTPUT_SIZE 32
#define DEFAULT_NAME_SIZE 8

#define MAX_PID_BYTES 16
#define MAX_EXEC_STATUS_BYTES 4
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

/** @brief Expand word with parameter substitution
 *  @param env Execution environment
 *  @param input Input word string
 *  @param input_types Array of character type flags
 *  @param output Output pointer for expanded string
 *  @return Status code
 */
StatusEnum expandWord(ExecuteEnvironmentPtr env, const char* input, const int8_t* input_types, char** output);