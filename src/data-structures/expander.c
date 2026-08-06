#include "data-structures/expander.h"

StatusEnum expanderCtor(ExpanderPtr exp, ExecuteEnvironmentPtr env, const char* input, const int8_t* input_types) {
    exp->current_input_pos = 0;
    exp->env = env;
    exp->input = input;
    exp->input_length = strlen(input);
    exp->input_types = input_types;

    StatusEnum st = stackCtor(&(exp->state_stack));
    ERR_CHECK(st);
    st = stackPush(&(exp->state_stack), EXP_NORMAL);
    ERR_CHECK(st);

    st = charBufferCtor(&(exp->output), DEFAULT_OUTPUT_SIZE);
    ERR_CHECK(st);
    st = charBufferCtor(&(exp->name), DEFAULT_NAME_SIZE);
    if(st != SUCCESS) {
        charBufferDtor(&(exp->output));
        return st;
    }

    return SUCCESS;
}


void expanderDtor(ExpanderPtr exp) {
    exp->current_input_pos = 0;
    exp->env = NULL;
    charBufferDtor(&(exp->output));
    charBufferDtor(&(exp->name));
}