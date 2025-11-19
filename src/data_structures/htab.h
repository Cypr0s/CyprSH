
#define HASH_TABLE_SIZE 64 // enough for env
#include "../util.h"

typedef struct htabitem {
    char* key;
    char* value;
    struct htabitem* next;
} HTabItem, *HTabItemPtr;

typedef HTabItemPtr HashTable[HASH_TABLE_SIZE];
typedef HashTable *HashTablePtr;

uint8_t getHash(const char* key);

void hashTableInit(HashTablePtr table);

ExitEnum hashTableInsert(HashTablePtr table, const char* key,const  char* value);

void hashTableRemove(HashTablePtr table, const char* key);

void hashTableDispose(HashTablePtr table);