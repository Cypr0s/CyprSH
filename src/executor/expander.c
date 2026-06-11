#include "executor/expander.h"

// expander
static StatusEnum expanderCtor(ExpanderPtr exp, ExecuteEnvironmentPtr env, const char* input, const int8_t* input_types);
static StatusEnum expanderDtor(ExpanderPtr exp);

// finite states handling
static StatusEnum handleNormal();
static StatusEnum handleDollar();
static StatusEnum handleTilde();

StatusEnum expandWord(ExecuteEnvironmentPtr env, const char* input, const int8_t* input_types, char** output) {
    Expander exp;
}

static StatusEnum expanderCtor(ExpanderPtr exp, ExecuteEnvironmentPtr env, const char* input, const int8_t* input_types) {
    return SUCCESS;
}

static StatusEnum expanderDtor(ExpanderPtr exp) {
    return SUCCESS;
}