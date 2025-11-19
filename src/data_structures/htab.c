#include "htab.h"

uint8_t getHash(const char* key) {
    uint32_t hash = 5381;
    int c;

    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }

    return (uint8_t)(hash & (HASH_TABLE_SIZE - 1));
}

void hashTableInit(HashTablePtr table) {
    for(uint8_t i = 0; i < HASH_TABLE_SIZE; i++) {
        (*table)[i] = NULL;
    }
}


char* hashTableGetValue(HashTablePtr table, const char* key) {
    uint8_t hash = getHash(key);
    HTabItemPtr tmp = (*table)[hash];

    while(tmp != NULL) {
        if(streq(tmp->key, key)) {
            return tmp->value;
        }
        tmp = tmp->next;
    }
    return NULL;
}


ExitEnum hashTableInsert(HashTablePtr table, const char* key, const char* value) {
    uint8_t hash = getHash(key);
    HTabItemPtr tmp = (*table)[hash];

    while(tmp != NULL) {
        if(streq(tmp->key, key)) {
            free(tmp->value);
            tmp->value = strdup(value);
            if(tmp->value == NULL) {
                customPrint();
                return EXIT_MALLOC_ERROR;
            }
            return EXIT_SUCCESS;
        }
        tmp = tmp->next;
    }

    HTabItemPtr new_item = (HTabItemPtr) malloc(sizeof(HTabItem));
    if(new_item == NULL) {
        customPrint();
        return EXIT_MALLOC_ERROR;
    }
    new_item->key = strdup(key);
    if(new_item->key == NULL) {
        customPrint();
        return EXIT_MALLOC_ERROR;
    }

    new_item->value = strdup(value);
    if(new_item->value == NULL) {
        customPrint();
        return EXIT_MALLOC_ERROR;
    }

    new_item->next = (*table)[hash];
    (*table)[hash] = new_item;
    return EXIT_SUCCESS;
}


void hashTableRemove(HashTablePtr table, const char* key) {
    uint8_t hash = getHash(key);
    HTabItemPtr tmp = (*table)[hash];
    if(tmp == NULL) return;

    if(streq(tmp->key, key)) {
        free(tmp->key);
        free(tmp->value);
        (*table)[hash] = tmp->next;
        free(tmp);
        return;
    }
    while(tmp->next != NULL) {
        if(streq(tmp->next->key, key)) {
            HTabItemPtr delete = tmp->next;
            tmp->next = delete->next;
            free(delete->key);
            free(delete->value);
            free(delete);
            return;
        }
        tmp = tmp->next;
    }
}


void hashTableDispose(HashTablePtr table) {
    HTabItemPtr tmp;
    for(uint8_t i = 0; i <HASH_TABLE_SIZE; i++) {
        tmp = (*table)[i];
        while(tmp) {
            (*table)[i] = tmp->next;
            free(tmp->key);
            free(tmp->value);
            free(tmp);
            tmp = (*table)[i];
        }
    }
}