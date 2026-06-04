/**
 * 
 */
#include "utils/env.h"


/**
 *  @brief
 */
StatusEnum populateEnvTable(HashTablePtr env_table, char** environ) {
    if(env_table == NULL || environ == NULL) {
        fprintf(stderr, "CyprSH: cannot initialize environment: invalid arguments\n");
        return ERROR_DEFAULT;
    }

    StatusEnum st = SUCCESS;
    // create hashtable
    st = hashTableCtor(env_table);
    ERR_CHECK(st);
    char* key;
    char* value;
    // loop through env variables
    while(*environ != NULL) {
        StatusEnum st = splitAssignment(*environ, &key, &value);
        if(st == ERROR_MALLOC_FAILURE) {
            fprintf(stderr, "CyprSH: out of memory while parsing environment\n");
            hashTableDtor(env_table);
            return st;
        } else if(st == ERROR_DEFAULT) {
            environ++;
            continue;
        }

        st = hashTableInsert(env_table, key, value);
        free(key);
        free(value);

        if(st != SUCCESS) {
            fprintf(stderr, "CyprSH: Failed to insert environment variable into hashtable\n");
            hashTableDtor(env_table);
            return st;
        }

        environ++;
    }
    return st;
}