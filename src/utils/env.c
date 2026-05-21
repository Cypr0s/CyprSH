/**
 * 
 */
#include "env.h"


/**
 *  @brief
 */
StatusEnum populateEnvTable(HashTablePtr env_table, char** environ) {
    if(env_table == NULL || environ == NULL) {
        return ERROR_DEFAULT;
    }

    StatusEnum st = SUCCESS;
    // create hashtable
    st = hashTableCtor(env_table);
    ERR_CHECK(st);
    // loop through env variables
    while(*environ != NULL) {
        char* eq = strchr(*environ, '=');

        if(eq == NULL) {
            environ++;
            continue;
        }

        size_t key_len = (size_t)(eq - *environ);
        char* key = strndup(*environ, key_len);
        if(key == NULL) {
            hashTableDtor(env_table);
            return ERROR_MALLOC_FAILURE;
        }

        st = hashTableInsert(env_table, key, eq + 1);
        free(key);

        if(st != SUCCESS) {
            hashTableDtor(env_table);
            return st;
        }

        environ++;
    }
    return st;
}