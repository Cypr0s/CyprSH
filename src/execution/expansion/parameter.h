#include "error.h"
#include "data-structures/hash-table.h"
#include "data-structures/expander.h"
#include "lexical-analysis/tokenize.h"

#ifndef PARAMETER_H
#define PARAMETER_H


typedef enum {
    EXPAND_PLAIN, // ${parameter}
    EXPAND_LENGTH, // ${#parameter}
    EXPAND_DEFAULT, // ${p:-word} / ${p-word}
    EXPAND_ASSIGN, // ${p:=word} / ${p=word}
    EXPAND_ERROR, // ${p:?word} / ${p?word}
    EXPAND_ALT, // ${p:+word} / ${p+word}
    EXPAND_REMOVE_SUFFIX_SMALL, // ${p%word}
    EXPAND_REMOVE_SUFFIX_LARGE, // ${p%%word}
    EXPAND_REMOVE_PREFIX_SMALL, // ${p#word}
    EXPAND_REMOVE_PREFIX_LARGE // ${p##word}
} ParameterOperationEnum;



typedef struct {
    ParameterOperationEnum op;
    size_t word_start;
    size_t parameter_start;
    size_t parameter_end;
    uint8_t has_colon;
    uint8_t has_word;
} ParameterExpansionState, *ParameterExpansionStatePtr;

StatusEnum expandParameter(ExpanderPtr exp);

#endif // PARAMETER_H