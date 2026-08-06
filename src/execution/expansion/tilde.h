#include "error.h"
#include "lexical-analysis/tokenize.h"
#include <pwd.h>
#include "data-structures/expander.h"

#ifndef TILDE_H
#define TILDE_H

StatusEnum expandTilde(ExpanderPtr exp);

#endif // TILDE_H