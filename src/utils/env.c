#include "env.h"


void populateEnvTable(HashTablePtr env_table, const char** environ) {
    hashTableInit(env_table);
    while(*environ != NULL) {
        const char* value = *environ; 
    
        while(*value != '\0' && *value != '=') {
            value++;
        }

        if((*value == '\0') || (*(value + 1)) == '\0') {
            environ++;
            continue;
        }

        uint32_t key_length = (uint32_t)( value - (*environ));

        char* key =(char*) malloc(key_length + 1);
        if(key == NULL) {
            hashTableDispose(env_table);
            error = EXIT_MALLOC_ERROR;
            return;
        }

        memcpy(key, *environ, key_length);
        key[key_length] = '\0';

        hashTableInsert(env_table, key, value + 1);
        if(error){
            free(key);
            hashTableDispose(env_table);
            return;
        }

        free(key);
        environ++;
    }
}