#include "utils/strings.h"
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


/**
 * @brief       Initializes open adressing resizable hashtable
 *              
 *              Function inits a resizable open addressing hashtable,
 *              allocates array for items, sets all indexes to ITEM_STATE_OPEN
 *              and all pointers to NULL
 * 
 * @param table pointer to HashTable structure which is passed by address
 * @return      int enum 0 on success, 3 on malloc failure(not eough memory)   
 */
StatusEnum hashTableCtor(HashTablePtr table);

/**
 * @brief       Destructor which frees all allocated memory
 *
 *              Hash table destructor which frees all allocated memory
 *              associated with hashtable by looping through its data,
 *              table MUST be reinitialized after destrucion before reuse
 * 
 * @param table pointer to HashTable structure which is passed by address
 * @return      nothing
 */
void hashTableDtor(HashTablePtr table);

/**
 * @brief       Inserts a item into hashtable based on hashed key
 *      
 *              Function for inserting item into hashtable based on hashed value of "key".
 *              If item with same key is already in hashtable the value of item is replaced
 *              and old one is trashed, If function has allocation failure or hash indexing failure
 *              corresponding exit values are returned otherwise 0(SUCCESS) is returned
 * 
 * @param table Pointer to hashtabnle structure in which item will be inserted
 * @param key   String value based on which position in the hashtable is decided
 *              its stored as String but position is based on double hashing open addressing
 * @param value String value associated with the key which will be inserted
 * @return      Int error exit codes if errors happen or success(0)
 */
StatusEnum hashTableInsert(HashTablePtr table, const char* key, const char* value);

/**
 * @brief       Resizes hash table to higher prime number and redistributes items
 *          
 *              FUnction increases the size of the hashtable to the next prime number
 *              while also re-inserting all FULL item indexes (DELETED and EMPTY indexes 
 *              are ignored)
 *              
 * 
 * @param table Pointer to hashtable which will be resized to next size
 * @return      error exit code 3 if allocation fails or 0 (SUCCESS)
 */
StatusEnum hashTableResize(HashTablePtr table);

/**
 * @brief       Deletes an item from hashtable based on the input key
 * 
 *              Function that removes an item from hashtable (frees all allocated structures) 
 *              based on the hash of input key. If no index is found or the index is empty 
 *              function does nothing.
 * 
 * @param table hashmap which holds indexes
 * @param key   key based on which index will be deleted
 * @return      nothing
 */
StatusEnum hashTableRemove(HashTablePtr table, const char* key);

/**
 * @brief       Gets value associated with the key from hashtable
 * 
 *             Function that gets the value associated with the key from hashtable, if key is not found
 *              or index is empty or deleted corresponding error codes are returned otherwise 0(SUCCESS) is returned
 */
StatusEnum hashTableGetValue(HashTablePtr table, char* key, char** value);
