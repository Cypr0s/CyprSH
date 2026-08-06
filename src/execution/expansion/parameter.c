#include "execution/expansion/parameter.h"

static uint8_t isEscapedOrQuoted(const char c);
static int8_t skipNestedOrQuoted(ExpanderPtr exp, size_t* i);
static StatusEnum parseBraceExpansion(ExpanderPtr exp);
static StatusEnum findBraceType(ExpanderPtr exp, ParameterExpansionStatePtr brace_exp);
static StatusEnum evalBraceExpansion(ExpanderPtr exp, ParameterExpansionStatePtr brace_exp);


static uint8_t isEscapedOrQouted(const char c) {
    return c == QUOTE_SINGLE_QUOTED || c == QUOTE_DOUBLE_QUOTED || c == QUOTE_ESCAPED;
}


static int8_t skipNestedOrQuoted(ExpanderPtr exp, size_t* i) {
    char c = exp->input[*i];

    if (isEscapedOrQouted(c)) {
        (*i)++;
        return 1;
    }

    if (c == '$' && *i + 1 < exp->input_length) {
        char next = exp->input[*i + 1];
        if (next == '{' || next == '(') {
            char open = next, close = (open == '{') ? '}' : ')';
            size_t j = *i + 2;
            int depth = 1;
            while (j < exp->input_length && depth > 0) {
                if (isEscapedOrQouted(exp->input[j])) { j++; continue; }
                if (exp->input[j] == open) depth++;
                else if (exp->input[j] == close) depth--;
                j++;
            }
            *i = j;
        } else {
            size_t j = *i + 1;
            if (isalpha((unsigned char)exp->input[j]) || exp->input[j] == '_') {
                while (j < exp->input_length &&
                       (isalnum((unsigned char)exp->input[j]) || exp->input[j] == '_'))
                    j++;
            } else if (j < exp->input_length) {
                j++;
            }
            *i = j;
        }
        return 1;
    }

    return 0;
}

static StatusEnum parseBraceExpansion(ExpanderPtr exp) {
    size_t i = exp->current_input_pos;
    charBufferReset(&(exp->name));
    while(i < exp->input_length) {
        if(skipNestedOrQuoted(exp, &i)) {
            continue;
        }
        if(exp->input[i] == '}') { 
            return SUCCESS;
        }
        charBufferAppendChar(&(exp->name), exp->input[i]);
        i++;
    }
    return ERROR_EXPANSION_FAILURE;
}

static StatusEnum findBraceType(ExpanderPtr exp, ParameterExpansionStatePtr brace_exp) {
    if(exp->name.size >= 2 && exp->name.buff[0] == '#') {
        size_t i = 1;
        if(isNameStart(exp->name.buff[i])) {
            i++;
            while(i < exp->name.size && isNameChar(exp->name.buff[i])) i++;
            if(i == exp->name.size) {
                brace_exp->op = EXPAND_LENGTH;
                brace_exp->parameter_start = 1;
                return SUCCESS;
            }
        } else if(exp->name.size == 2) {
            brace_exp->op = EXPAND_LENGTH;
            brace_exp->parameter_start = 1;
            return SUCCESS;
        }
    }

    size_t i = 0;
    if(isNameStart(exp->name.buff[i])) {
        i++;
        while(i < exp->name.size && isNameChar(exp->name.buff[i])) i++;
    } else if (i < exp->name.size){
        i++;
    }

    if(i == exp->name.size) {
        brace_exp->op = EXPAND_PLAIN;
        brace_exp->parameter_start = 0;
        return SUCCESS;
    }

    brace_exp->parameter_end = i;

    if(exp->name.buff[i] == ':') {
        brace_exp->has_colon = 1;
        i++;
        if (i >= exp->name.size) {
            return ERROR_EXPANSION_FAILURE;
        }
        switch (exp->name.buff[i]) {
            case '-':
                brace_exp->op = EXPAND_DEFAULT;
                break;
            case '=':
                brace_exp->op = EXPAND_ASSIGN;
                break;
            case '?': 
                brace_exp->op = EXPAND_ERROR;
                break;
            case '+': 
                brace_exp->op = EXPAND_ALT;
                break;
            default:  
                return ERROR_EXPANSION_FAILURE;
        }
        i++;
    } else {
        brace_exp->has_colon = 0;
        switch (exp->name.buff[i]) {
            case '-': 
                brace_exp->op = EXPAND_DEFAULT;
                break;
            case '=': 
                brace_exp->op = EXPAND_ASSIGN;
                break;
            case '?':
                brace_exp->op = EXPAND_ERROR;
                break;
            case '+': 
                brace_exp->op = EXPAND_ALT;
                break;
            case '%': {
                if(i + 1 < exp->name.size && exp->name.buff[i + 1] == '%') {
                    brace_exp->op = EXPAND_REMOVE_SUFFIX_LARGE;
                    i++;
                } else {
                    brace_exp->op = EXPAND_REMOVE_SUFFIX_SMALL;
                }
                break;
            }
            case '#': {
                if(i + 1 < exp->name.size && exp->name.buff[i + 1] == '#') {
                    brace_exp->op = EXPAND_REMOVE_PREFIX_LARGE;
                    i++;
                } else {
                    brace_exp->op = EXPAND_REMOVE_PREFIX_SMALL;
                }
                break;
            }
            default:
                return ERROR_EXPANSION_FAILURE;
        }
        i++;
    }
    brace_exp->word_start = i;
    return SUCCESS;
}



static StatusEnum evalBraceExpansion(ExpanderPtr exp, ParameterExpansionStatePtr brace_exp) {
    char* parameter = NULL;
    const char* param_name = exp->name.buff + brace_exp->parameter_start;
    size_t param_name_len = brace_exp->parameter_end - brace_exp->parameter_start;

    char saved = exp->name.buff[brace_exp->parameter_end];
    exp->name.buff[brace_exp->parameter_end] = '\0';

    StatusEnum st = hashTableGetValue(exp->env->env_table, param_name, &parameter);
    exp->name.buff[brace_exp->parameter_end] = saved; // restore immediately after lookup

    if (st != ERROR_HTAB_ITEM && st != SUCCESS) {
        return st;
    }

    int8_t is_set = (st == SUCCESS);
    int8_t is_empty = is_set && parameter[0] == '\0';
    int8_t trigger = brace_exp->has_colon ? (is_set && !is_empty) : is_set;

    // word runs [word_start, exp->name.size) — relies on charBuffer null-terminating at .size
    const char* word = exp->name.buff + brace_exp->word_start;

    switch (brace_exp->op) {

        case EXPAND_PLAIN:
            if (is_set) {
                st = charBufferAppendCharPtr(&(exp->output), parameter, strlen(parameter));
                ERR_CHECK(st);
            }
            break;

        case EXPAND_LENGTH:
            // TODO: if set -u is in effect and !is_set, error out here
            if (is_set) {
                char buff[32];
                int n = snprintf(buff, sizeof(buff), "%zu", strlen(parameter));
                st = charBufferAppendCharPtr(&(exp->output), buff, (size_t)n);
                ERR_CHECK(st);
            }
            break;

        case EXPAND_DEFAULT:
            if (trigger) {
                st = charBufferAppendCharPtr(&(exp->output), parameter, strlen(parameter));
                ERR_CHECK(st);
            } else if (brace_exp->has_word) {
                // TODO: run `word` through expandWord (tilde/param/cmd/arith + quote removal)
                st = charBufferAppendCharPtr(&(exp->output), word, strlen(word));
                ERR_CHECK(st);
            }
            break;

        case EXPAND_ASSIGN:
            if (trigger) {
                st = charBufferAppendCharPtr(&(exp->output), parameter, strlen(parameter));
                ERR_CHECK(st);
            } else {
                if (!isAssignableName(param_name, param_name_len)) {
                    printError("evalBraceExpansion",
                               "%.*s: cannot assign in this way", (int)param_name_len, param_name);
                    return ERROR_EXPANSION_FAILURE;
                }
                const char* value = brace_exp->has_word ? word : "";
                // TODO: expand `word` (if present) + quote removal before assigning/appending
                st = hashTableInsert(exp->env->env_table, param_name, value);
                ERR_CHECK(st);
                st = charBufferAppendCharPtr(&(exp->output), value, strlen(value));
                ERR_CHECK(st);
            }
            break;

        case EXPAND_ERROR:
            if (trigger) {
                st = charBufferAppendCharPtr(&(exp->output), parameter, strlen(parameter));
                ERR_CHECK(st);
            } else if (brace_exp->has_word) {
                printError("evalBraceExpansion", "%s", word);
                return ERROR_EXPANSION_FAILURE;
            } else {
                printError("evalBraceExpansion", "%.*s: parameter null or not set",
                           (int)param_name_len, param_name);
                return ERROR_EXPANSION_FAILURE;
            }
            break;

        case EXPAND_ALT:
            if (trigger && brace_exp->has_word) {
                st = charBufferAppendCharPtr(&(exp->output), word, strlen(word));
                ERR_CHECK(st);
            }
            break;

        case EXPAND_REMOVE_SUFFIX_SMALL:
        case EXPAND_REMOVE_SUFFIX_LARGE:
        case EXPAND_REMOVE_PREFIX_SMALL:
        case EXPAND_REMOVE_PREFIX_LARGE:
            // not implemented yet
            break;

        default:
            return ERROR_EXPANSION_FAILURE;
    }
    return SUCCESS;
}


StatusEnum expandParameter(ExpanderPtr exp) {
    ParameterExpansionState brace_exp;
    memset(&brace_exp, 0, sizeof(brace_exp));
    brace_exp.parameter_start = exp->current_input_pos;

    // copy slice
    StatusEnum st = parseBraceExpansion(exp);
    ERR_CHECK(st);

    st = findBraceType(exp, &brace_exp);
    ERR_CHECK(st);

    st = evalBraceExpansion(exp, &brace_exp);
    ERR_CHECK(st);

    return stackPop(&(exp->state_stack));
}