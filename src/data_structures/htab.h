#include "../utils/strings.h"
#include "../utils/error.h"

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
 * @brief   Computes a 32-bit FNV-1a hash of a string
 * @param s pointer to a string which will be hashed
 * @return  32bit hashed value from input string
 */
uint32_t hash1(const char *key);

/**
 * @brief   Computes a 32-bit Jenkins-style hash of a string
 * @param s pointer to a string which will be hashed
 * @return  32bit hashed value from input string
 */
uint32_t hash2(const char *key);

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
 * @brief       Finds corresponding index of key in hashmap
 *          
 *              Function that calculates corresponding index of key based on its two hashes,
 *              if position is FULL or DESTROYED it moves to next index until it finds a open one.
 *              If key is already in hashtable it returns the index where its located
 *
 * @param table Hash table in which the index will be searched for
 * @param key   String that corresponds to the index
 * @return      Int position(index) where key should be inserted or is located(insertion/deletion)
 *              -1 if index is not found
 */
static int32_t hashTableFindIndex(HashTablePtr table, const char* key);

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
 *  @brief      Finds next higher prime of input num from hashtable_prime_capacities
 *
 *              Function thaht finds the next higher prime number from input number and 
 *              hashtable_prime_capacities, looking up next primes with binary search, 
 *              if the number needs to be higher or is already higher it looks one up by
 *              by looping       
 * 
 *  @param  num num based on which next size of hashtable will be decided
 *  @return     int default error 1 if passed a NULL, 3 if integer overflow happens
 */
static StatusEnum hashTableNextPrime(uint32_t* num);

/**
 * @brief  Finds the smallest prime number greater than num
 *
 * @param  num  Starting value for the prime search
 * @return The closest prime number > num
 */
static uint32_t closestHigherPrime(uint32_t num);

/**
 * @brief  Tests whether a number is prime
 *
 * @param  n  Number to test
 * @return 1 if n is prime, 0 otherwise
 */
static uint8_t isPrime(uint32_t n);