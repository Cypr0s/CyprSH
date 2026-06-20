#ifndef FUNCTION_LIST_H
#define FUNCTION_LIST_H

#include "error.h"
#include "data_structures/ast.h"

typedef struct FunctionEntry {
    char* name;
    ASTNodePtr body;
    struct FunctionEntry* next;
} FunctionEntry, *FunctionEntryPtr;

typedef struct {
    FunctionEntryPtr head;
} FunctionList, *FunctionListPtr;

void functionListCtor(FunctionListPtr fl);

void functionListDtor(FunctionListPtr fl);

StatusEnum functionListInsert(FunctionListPtr fl, const char* name, ASTNodePtr body);

ASTNodePtr functionListFind(FunctionListPtr fl, const char* name);

#endif