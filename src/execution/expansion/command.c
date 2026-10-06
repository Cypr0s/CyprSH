#include "execution/expansion/command.h"


StatusEnum expandCommandSub(ExpanderPtr exp) {
    (void)exp;
    // TODO: command substitution $(cmd) not implemented yet
    return ERROR_DEFAULT;
}