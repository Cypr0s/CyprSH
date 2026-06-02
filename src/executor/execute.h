#ifndef EXECUTE_H
#define EXETUCE_H

#include "data_structures/ast.h"
#include "error.h"
#include "utils/env.h"

StatusEnum executeNode(ASTNodePtr node);

#endif // EXECUTE_H