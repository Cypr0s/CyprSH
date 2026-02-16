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
        char* value = strchr(*environ, '=');

        if(value == NULL || *value == '\0') {
            environ++;
            continue;
        }
        // insert into hashmap
        *value = '\0';
        st = hashTableInsert(env_table, *environ, value + 1);
        *value = '=';

        if(st != SUCCESS){
            hashTableDtor(env_table);
            return st;
        }

        environ++;
    }
    return st;
}