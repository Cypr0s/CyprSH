#include "env.h"


ExitEnum populateEnvTable(HashTablePtr env_table,const char** environ) {
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
            return EXIT_MALLOC_ERROR;
        }

        memcpy(key, *environ, key_length);
        key[key_length] = '\0';

        if(hashTableInsert(env_table, key, value + 1)){
            free(key);
            hashTableDispose(env_table);
            return EXIT_MALLOC_ERROR;
        }

        free(key);
        environ++;
    }
    return EXIT_SUCCESS;
}