#ifndef COMMAND_H
#define COMMAND_H

#include "error.h"
#include "data-structures/expander.h"


StatusEnum expandCommandSub(ExpanderPtr exp);

#endif // COMMAND_H