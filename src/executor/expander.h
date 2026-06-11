#include "error.h"
#include "execute_types.h"
#include "data_structures/stack.h"
#include "data_structures/char_buffer.h"

typedef enum {
    EXP_NORMAL, // unqoted chars
    EXP_TILDE, // ~
    EXP_DOLLAR, // $

} ExpanderStateEnum;

typedef struct {
    char* input;
    int8_t input_types;
    size_t input_length;
    size_t current_input_pos;
    ExecuteEnvironmentPtr env;
    Stack state_stack;
    CharBuffer output;
    CharBuffer name;
} Expander, *ExpanderPtr;

StatusEnum expandWord(ExecuteEnvironmentPtr env, const char* input, const int8_t* input_types, char** output);