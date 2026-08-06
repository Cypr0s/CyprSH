#ifndef HTAB_H
#define HTAB_H

#include "utilities/strings.h"
#include "error.h"

//
typedef enum {
    ITEM_STATE_EMPTY,
    ITEM_STATE_FULL,
    ITEM_STATE_DELETED
} HtabState;


//
typedef struct {
    char* key;
    char* value;
    HtabState state;
} HashTableItem, *HashTableItemPtr;


//
typedef struct hashtable {
    HashTableItemPtr data;
    uint32_t currentSize;
    uint32_t capacity;
} HashTable, *HashTablePtr;


typedef struct {
    HashTablePtr table;
    uint32_t index;
} HashTableIter, *HashTableIterPtr;

/** @brief Initialize open addressing resizable hash table
 *  @param table Hash table to initialize
 *  @return Status code (SUCCESS or ERROR_MALLOC_FAILURE)
 */
StatusEnum hashTableCtor(HashTablePtr table);

/** @brief Destroy hash table and free all memory
 *  @param table Hash table to destroy
 */
void hashTableDtor(HashTablePtr table);

/** @brief Insert or update key-value pair
 *  @param table Target hash table
 *  @param key String key
 *  @param value String value to insert
 *  @return Status code
 */
StatusEnum hashTableInsert(HashTablePtr table, const char* key, const char* value);

/** @brief Resize hash table to next prime capacity
 *  @param table Hash table to resize
 *  @return Status code (SUCCESS or ERROR_MALLOC_FAILURE)
 */
StatusEnum hashTableResize(HashTablePtr table);

/** @brief Delete key-value pair from hash table
 *  @param table Hash table
 *  @param key Key to remove
 */
StatusEnum hashTableRemove(HashTablePtr table, const char* key);

/** @brief Get value associated with key
 *  @param table Hash table to search
 *  @param key Key to look up
 *  @param value Output pointer for value string
 *  @return Status code
 */
StatusEnum hashTableGetValue(HashTablePtr table, char* key, char** value);

/** @brief Get current size of hash table
 *  @param table Hash table
 *  @return Current item count
 */
uint32_t hashTableGetCurrSize(HashTablePtr table);

/** @brief Initialize hash table iterator
 *  @param table_iterator Iterator to initialize
 *  @param table Source hash table
 */
void hashTableIterCtor(HashTableIterPtr table_iterator, HashTablePtr table);

/** @brief Get next key-value pair from iterator
 *  @param table_iterator Source iterator
 *  @param key Output pointer for key
 *  @param value Output pointer for value
 *  @return 1 if valid item retrieved, 0 if end of table
 */
uint8_t hashTableIterNext(HashTableIterPtr table_iterator, char** key, char** value);

#endif // HTAB_H
