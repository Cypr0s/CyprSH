#include "data_structures/function_list.h"


void functionListCtor(FunctionListPtr fl) {
    fl->head = NULL;
}


void functionListDtor(FunctionListPtr fl) {
    if(fl == NULL) return;

    FunctionEntryPtr current = fl->head;
    while(current != NULL) {
        FunctionEntryPtr next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    fl->head = NULL;
}


StatusEnum functionListInsert(FunctionListPtr fl, const char* name, ASTNodePtr body) {
    if(fl == NULL || name == NULL) {
        printError("functionListInsert", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }

    FunctionEntryPtr current = fl->head;
    while(current != NULL) {
        if(streq(current->name, name)) {
            current->body = body;
            return SUCCESS;
        }
        current = current->next;
    }

    FunctionEntryPtr entry = malloc(sizeof(FunctionEntry));
    if(entry == NULL) {
        printError("functionListInsert", "Malloc failure");
        return ERROR_MALLOC_FAILURE;
    }

    entry->name = strdup(name);
    if(entry->name == NULL) {
        free(entry);
        printError("functionListInsert", "Malloc failure");
        return ERROR_MALLOC_FAILURE;
    }

    entry->body = body;
    entry->next = fl->head;
    fl->head = entry;

    return SUCCESS;
}


ASTNodePtr functionListFind(FunctionListPtr fl, const char* name) {
    if(fl == NULL || name == NULL) {
        return NULL;
    }

    FunctionEntryPtr current = fl->head;
    while(current != NULL) {
        if(streq(current->name, name)) {
            return current->body;
        }
        current = current->next;
    }
    return NULL;
}