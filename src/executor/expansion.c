#include "executor/expansion.h"

// expander
static StatusEnum expanderCtor(ExpanderPtr exp, ExecuteEnvironmentPtr env, const char* input, const int8_t* input_types);
static void expanderDtor(ExpanderPtr exp);

// finite states handling
static StatusEnum handleNormal(ExpanderPtr exp);
static StatusEnum handleDollar(ExpanderPtr exp);
static StatusEnum handleTilde(ExpanderPtr exp);

StatusEnum expandWord(ExecuteEnvironmentPtr env, const char* input, const int8_t* input_types, char** output) {
    Expander exp;
    int8_t state_val;
    StatusEnum st = expanderCtor(&exp, env, input, input_types);
    ERR_CHECK(st);

    while(exp.current_input_pos < exp.input_length) {
        st = stackTop(&(exp.state_stack), &state_val);

        if(st != SUCCESS) {
            expanderDtor(&exp);
            return st;
        }

        ExpanderStateEnum state = (ExpanderStateEnum) state_val;

        switch(state) {
            case EXP_NORMAL:
                st = handleNormal(&exp);
                break;
            case EXP_DOLLAR:
                st = handleDollar(&exp);
                break;
            case EXP_TILDE:
                st = handleTilde(&exp);
                break;
            default:
                st = ERROR_DEFAULT;
            break;
        }

        if(st != SUCCESS) {
            expanderDtor(&exp);
            return st;
        }
    } 

    expanderDtor(&exp);
    return st;
}


static StatusEnum expanderCtor(ExpanderPtr exp, ExecuteEnvironmentPtr env, const char* input, const int8_t* input_types) {
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


static void expanderDtor(ExpanderPtr exp) {
    exp->current_input_pos = 0;
    exp->env = NULL;
    charBufferDtor(&(exp->output));
    charBufferDtor(&(exp->name));
}