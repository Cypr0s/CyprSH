/**
 * @file        expansion.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Parameter and variable expansion implementation
 */

#include "execution/expansion/expansion.h"

// expander
static StatusEnum expanderCtor(ExpanderPtr exp, ExecuteEnvironmentPtr env, const char* input, const int8_t* input_types);
static void expanderDtor(ExpanderPtr exp);

// finite states handling
static StatusEnum expandNormal(ExpanderPtr exp);
static StatusEnum handleDollar(ExpanderPtr exp);


static StatusEnum expandNormal(ExpanderPtr exp) {
    char c = exp->input[exp->current_input_pos];
    int8_t type = exp->input_types[exp->current_input_pos];
    // any dollar
    if(c == '$' && (type == QUOTE_UNQUOTED || type == QUOTE_DOUBLE_QUOTED)) {
        exp->current_input_pos++;
        return stackPush(&(exp->state_stack), EXP_DOLLAR);
    }
    // tilde at start
    if(c == '~' && exp->current_input_pos == 0 && type == QUOTE_UNQUOTED) {
        exp->current_input_pos++;
        return stackPush(&(exp->state_stack), EXP_TILDE);

    }

    StatusEnum st = charBufferAppendChar(&(exp->output), c);
    ERR_CHECK(st);
    exp->current_input_pos++;
    return SUCCESS;
}


static StatusEnum handleDollar(ExpanderPtr exp) {
    if(exp->current_input_pos >= exp->input_length) {
        StatusEnum st = charBufferAppendChar(&(exp->output), '$');
        ERR_CHECK(st);
        return stackPop(&(exp->state_stack));
    }

    switch(exp->input[exp->current_input_pos]) {
        case '{': {
            // brace replace DOLLAR with BRACE
            exp->current_input_pos++;
            StatusEnum st = stackPop(&(exp->state_stack));
            ERR_CHECK(st);
            return stackPush(&(exp->state_stack), EXP_BRACE);
        }
        case '$': {
            //process id
            exp->current_input_pos++;
            char buf[MAX_PID_BYTES];
            snprintf(buf, sizeof(buf), "%ld", (long)exp->env->shell_pid); // should be enough for pid
            StatusEnum st = charBufferAppendCharPtr(&(exp->output), buf, strlen(buf));
            ERR_CHECK(st);
            return stackPop(&(exp->state_stack));
        }

        case '?': {
            // last status
            exp->current_input_pos++;
            char buf[MAX_EXEC_STATUS_BYTES];
            snprintf(buf, sizeof(buf), "%d", exp->env->last_exec_status);
            StatusEnum st = charBufferAppendCharPtr(&(exp->output), buf, strlen(buf));
            ERR_CHECK(st);
            return stackPop(&(exp->state_stack));
        }

        case '!': {
            // last bg pid
            exp->current_input_pos++;
            if(exp->env->last_bg_pid > 0) {
                char buf[MAX_PID_BYTES];
                snprintf(buf, sizeof(buf), "%ld", (long)exp->env->last_bg_pid);
                StatusEnum st = charBufferAppendCharPtr(&(exp->output), buf, strlen(buf));
                ERR_CHECK(st);
            }
            return stackPop(&(exp->state_stack));
        }

        case '#': {
            // arg count
            exp->current_input_pos++;
            char buf[MAX_EXEC_STATUS_BYTES];
            snprintf(buf, sizeof(buf), "%d", exp->env->arguments_count);
            StatusEnum st = charBufferAppendCharPtr(&(exp->output), buf, strlen(buf));
            ERR_CHECK(st);
            return stackPop(&(exp->state_stack));
        }

        case '@':
        case '*': {
            // all args
            exp->current_input_pos++;
            for(int16_t i = 0; i < exp->env->arguments_count; i++) {
                if(i > 0) {
                    StatusEnum st = charBufferAppendChar(&(exp->output), ' ');
                    ERR_CHECK(st);
                }
                char* param = exp->env->arguments[i];
                StatusEnum st = charBufferAppendCharPtr(&(exp->output), param, strlen(param));
                ERR_CHECK(st);
            }
            return stackPop(&(exp->state_stack));
        }

        case '(': {
            // replace DOLLAR with ARITHMETIC/COMMAND_SUB
            exp->current_input_pos++;
            StatusEnum st = stackPop(&(exp->state_stack));
            ERR_CHECK(st);

            // $(()) arithmetic, $() command substitution
            if(exp->current_input_pos < exp->input_length &&
                exp->input[exp->current_input_pos] == '('
            ) {
                exp->current_input_pos++;
                return stackPush(&(exp->state_stack), EXP_ARITHMETIC);

            }

            return stackPush(&(exp->state_stack), EXP_COMMAND_SUB);
        }

        default:
            break;
    }

    char c = exp->input[exp->current_input_pos];

    if(isdigit((unsigned char)c)) {
        exp->current_input_pos++;
        int index = c - '0';
        if(index < exp->env->arguments_count) {
            char* param = exp->env->arguments[index];
            StatusEnum st = charBufferAppendCharPtr(&(exp->output), param, strlen(param));
            ERR_CHECK(st);
        }
        // empty string
        return stackPop(&(exp->state_stack));
    }

    // invalid char -> literal $
    if(!isalpha((unsigned char)c) && c != '_') {
        StatusEnum st = charBufferAppendChar(&(exp->output), '$');
        ERR_CHECK(st);
        return stackPop(&(exp->state_stack));
    }

    // $VAR
    charBufferReset(&(exp->name));
    while(exp->current_input_pos < exp->input_length) {
        c = exp->input[exp->current_input_pos];
        if(!isalnum((unsigned char)c) && c != '_') {
            break;
        }
        StatusEnum st = charBufferAppendChar(&(exp->name), c);
        ERR_CHECK(st);
        exp->current_input_pos++;
    }

    char* value = NULL;
    StatusEnum lookup_st = hashTableGetValue(exp->env->env_table, exp->name.buff, &value);

    if(lookup_st == SUCCESS && value != NULL) {
        StatusEnum st = charBufferAppendCharPtr(&(exp->output), value, strlen(value));
        ERR_CHECK(st);
    }

    return stackPop(&(exp->state_stack));
}

static StatusEnum handleArithmetic(ExpanderPtr exp) {
    (void)exp;
    // TODO: arithmetic expansion $((expr)) not implemented yet
    return ERROR_DEFAULT;
}

static StatusEnum handleCommandSub(ExpanderPtr exp) {
    (void)exp;
    // TODO: command substitution $(cmd) not implemented yet
    return ERROR_DEFAULT;
}


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
            case EXP_ARITHMETIC:
                st = handleArithmetic(&exp);
                break;
            case EXP_COMMAND_SUB:
                st = handleCommandSub(&exp);
                break;
            case EXP_BRACE:
                st = handleBrace(&exp);
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

    // transfer output
    *output = charBufferTransfer(&(exp.output));
    if(*output == NULL) {
        expanderDtor(&exp);
        return ERROR_DEFAULT;
    }

    expanderDtor(&exp);
    return st;
}