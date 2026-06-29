/**
 * @file        env.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Environment variable table initialization implementation
 */

#include "utils/env.h"

/** @brief Populate environment table from system environ
 *  @param env_table Hash table to populate
 *  @param environ System environment variables array
 *  @return Status code
 */
StatusEnum populateEnvTable(HashTablePtr env_table, char** environ) {
    if(env_table == NULL || environ == NULL) {
        printError("populateEnvTable", "Passing NULL pointers");
        return ERROR_DEFAULT;
    }

    StatusEnum st = hashTableCtor(env_table);
    ERR_CHECK(st);

    char* key;
    char* value;

    while(*environ != NULL) {
        st = splitAssignment(*environ, &key, &value);
        if(st == ERROR_MALLOC_FAILURE) {
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
            hashTableDtor(env_table);
            return st;
        }

        environ++;
    }
    return st;
}