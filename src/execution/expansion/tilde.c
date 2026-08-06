#include "executor/expansion/tilde.h"

StatusEnum expandTilde(ExpanderPtr exp) {
    charBufferReset(&(exp->name));
    // all characters until unqoted '/'
    while(exp->current_input_pos < exp->input_length) {
        if(exp->input[exp->current_input_pos]== '/' && 
            (QuoteTypeEnum) exp->input_types[exp->current_input_pos] == QUOTE_UNQUOTED
        ) {
            break;
        }

        StatusEnum st = charBufferAppendChar(&(exp->name), exp->input[exp->current_input_pos]);
        ERR_CHECK(st);
        exp->current_input_pos++;
    }

    char* replacement = NULL;

    if(exp->name.size == 0) {
        char* home = NULL;
        StatusEnum lookup_st = hashTableGetValue(exp->env->env_table, "HOME", &home);
        // not found or deleted
        if(lookup_st == ERROR_HTAB_ITEM) {
            StatusEnum st = charBufferAppendChar(&(exp->output), '~');
            ERR_CHECK(st);
            return stackPop(&(exp->state_stack));
        }
        ERR_CHECK(lookup_st); 

        replacement = home;

    } else {
        // usernnam
        struct passwd* user_info = getpwnam(exp->name.buff);
        if(user_info == NULL) {
            StatusEnum st = charBufferAppendChar(&(exp->output), '~');
            ERR_CHECK(st);
            st = charBufferAppendCharPtr(&(exp->output), exp->name.buff, exp->name.size);
            ERR_CHECK(st);
            return stackPop(&(exp->state_stack));
            
        }
        replacement = user_info->pw_dir;
    }

    size_t replacement_len = strlen(replacement);
    uint8_t next_slash = (exp->current_input_pos < exp->input_length &&
                            exp->input[exp->current_input_pos] == '/'
                        );

    if(next_slash && replacement_len > 0 && replacement[replacement_len - 1] == '/') {
        replacement_len--;
    }

    if(replacement_len > 0) { // HOME=""
        StatusEnum st = charBufferAppendCharPtr(&(exp->output), replacement, replacement_len);
        ERR_CHECK(st);
    }

    return stackPop(&(exp->state_stack));
}