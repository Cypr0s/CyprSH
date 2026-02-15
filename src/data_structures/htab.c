/**
 * 
 */

#include "htab.h"

// Load factor threshold constants used resizing when table is 11/16 full
#define LOAD_FACTOR_NUM 11
#define LOAD_FACTOR_DENUM 16

// closest lower prime number to UINT32MAX used as upper boundary for hashtable size
#define CLOSEST_UMAX32_PRIME 4294967291U

/*  Hash table prime number capacities which will be
    used for resizing the created hashtable(s),
    hashtable will be resized to next index when its
    current size is higher than 0.675  (11/16).
    Array is also in ascending order for function
    for hashTableNextPrime!! if you want to add more 
    primes they must be correctly ordered!  
*/
static const uint32_t hashtable_prime_capacities[] = {
    31, 61, 127, 193, 389, 769, 1543,
    3079, 6151, 12289, 24593, 49157, 98317
};


/**
 * @brief   Computes a 32-bit FNV-1a hash of a string
 * @param s pointer to a string which will be hashed
 * @return  32bit hashed value from input string
 */
uint32_t hash1(const char *key) {
    uint32_t h = 2166136261U;

    while (*key) {
        h ^= (uint8_t)*key++;
        h *= 16777619U;
    }

    return h;
} // hash1


/**
 * @brief   Computes a 32-bit Jenkins-style hash of a string
 * @param s pointer to a string which will be hashed
 * @return  32bit hashed value from input string
 */
uint32_t hash2(const char *key) {
    uint32_t h = 0;

    while (*key) {
        h += (uint8_t)*key++;
        h += (h << 10);
        h ^= (h >> 6);
    }

    h += (h << 3);
    h ^= (h >> 11);
    h += (h << 15);

    return h;
} // hash2


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
StatusEnum hashTableCtor(HashTablePtr table) {
    if(table == NULL) {
        return ERROR_DEFAULT;
    }

    memset(table, 0, sizeof(*table));

    // initialize hashtable
    table->capacity = hashtable_prime_capacities[0]; // select first prime number from array
    table->currentSize = 0;
    table->data = (HashTableItemPtr) malloc(sizeof(HashTableItem) * table->capacity);
    if(table->data == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    // reset all array positions, set them to EMPTY
    for(uint32_t i = 0; i < table->capacity; i++) {
        table->data[i].state = ITEM_STATE_EMPTY;
        table->data[i].key = NULL;
        table->data[i].value = NULL;
    }

    return SUCCESS;
} // hashTableInit


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
void hashTableDtor(HashTablePtr table) {
    if(table == NULL || table->data == NULL) {
        return;
    }

    // loop through whole hashtable and free occupied indexes
    for(uint32_t i = 0; i < table->capacity; i++) {
        HashTableItemPtr item = &(table->data[i]);
        if(item->state == ITEM_STATE_FULL) {
            free(item->key);
            free(item->value);
        } // if
    } // for

    // free table->data allocated array
    free(table->data);
    table->data = NULL;
    // optional
    table->currentSize = 0;
    table->capacity = 0;
    return;
} // HashTableDispose


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
StatusEnum hashTableInsert(HashTablePtr table, const char* key, const char* value) {

    if(table == NULL || table->data == NULL || key == NULL || value == NULL) {
        return ERROR_DEFAULT;
    }
        

    /* if hashtable is overloaded ( higher load than 0.675) 
        increase size to closest higher prime number
    */

    if((table->currentSize + 1) * LOAD_FACTOR_DENUM >= table->capacity * LOAD_FACTOR_NUM) {
        StatusEnum st = hashTableResize(table);
        if(st != SUCCESS) {
            return st;
        }
    }

    int32_t index = hashTableFindIndex(table, key);
    if(index == -1) {
        return ERROR_INDEX_OUT_OF_BOUNDS;
    }

    HashTableItemPtr item = &(table->data[index]);

    char *newValue = strdup(value);
    if (newValue == NULL)
        return ERROR_MALLOC_FAILURE;

    // rewrite value if the key is already in hashtable
    if(item->state == ITEM_STATE_FULL) {
        free(item->value);
        item->value = newValue;
        return SUCCESS;
    }

    // key is stored as string not hashed int
    item->key = strdup(key);
    if (item->key == NULL) {
        free(newValue);
        return ERROR_MALLOC_FAILURE;
    }

    item->value = newValue;
    item->state = ITEM_STATE_FULL;
    table->currentSize++;

    return SUCCESS;
} // hashTableInsert


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
StatusEnum hashTableResize(HashTablePtr table) {
    uint32_t old_capacity = table->capacity;

    StatusEnum st = hashTableNextPrime(&(table->capacity));
    if(st != SUCCESS) {
        return st;
    }

    HashTableItemPtr new_data = malloc(sizeof(HashTableItem) * table->capacity);
    if(new_data == NULL) {
        table->capacity = old_capacity;
        return ERROR_MALLOC_FAILURE;
    }

    HashTableItemPtr old_data = table->data;
    // default state to all new indexes
    table->data = new_data;
    for (uint32_t i = 0; i < table->capacity; i++) {
        table->data[i].state = ITEM_STATE_EMPTY;
        table->data[i].key = NULL;
        table->data[i].value = NULL;
    }

    // reenter all full indexes

    uint32_t new_table_index;
    for(uint32_t i = 0; i < old_capacity; i++) {
        if(old_data[i].state == ITEM_STATE_FULL) {
            new_table_index = hashTableFindIndex(table, old_data[i].key);
            table->data[new_table_index].state = ITEM_STATE_FULL;
            table->data[new_table_index].key = old_data[i].key;
            table->data[new_table_index].value = old_data[i].value;
        } // if
    } // for

    free(old_data);
    return SUCCESS;
} // hashTableResize


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
static int32_t hashTableFindIndex(HashTablePtr table, const char* key) {
    int32_t table_index;

    // calculate hashes of key
    uint32_t base_index = hash1(key) % table->capacity;
    uint32_t step = (hash2(key) % (table->capacity - 1)) + 1;
    int32_t first_deleted = -1;
    // loop through every index of table (guaranteed because of a prime number)
    for(uint32_t i = 0; i < table->capacity; i++) {
        // calculate the correct index
        table_index = (base_index + i * step) % table->capacity;

        HashTableItemPtr item = &(table->data[table_index]);

        if(item->state == ITEM_STATE_DELETED) {
            if(first_deleted == -1) {
                first_deleted = table_index;
            }
        }
        // if its not in hashtable replace it with first occurence
        if(item->state == ITEM_STATE_EMPTY) {
            return (first_deleted != -1 ? first_deleted : table_index);
        }
    
        // if its found return it
        if(item->state == ITEM_STATE_FULL && strcmp(key, item->key) == 0) {
            return table_index;
        }
    }
    return -1; // index not found
} // hashTableFindIndex


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
StatusEnum hashTableRemove(HashTablePtr table, const char* key) {
    if(table == NULL || key == NULL || table->data == NULL) {
        return ERROR_DEFAULT;
    }

    int32_t index = hashTableFindIndex(table, key);

    // index was not found
    if(index == -1) {
        return SUCCESS;
    }

    HashTableItemPtr item = &(table->data[index]);
    // index was deleted or empty
    if(item->state == ITEM_STATE_DELETED || item->state == ITEM_STATE_EMPTY) {
        return SUCCESS;
    }
    // free key and value
    if(item->key != NULL) {
        free(item->key);
    }
    if(item->value != NULL) {
        free(item->value);
    }

    item->key = NULL;
    item->value = NULL;
    table->currentSize--;
    item->state = ITEM_STATE_DELETED;
    return SUCCESS;
} // hashTableRemove 


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
static StatusEnum hashTableNextPrime(uint32_t* num) {
    if(num == NULL) {
        return ERROR_DEFAULT;
    }
    uint32_t count = sizeof(hashtable_prime_capacities) / sizeof(hashtable_prime_capacities[0]);
    uint32_t l = 0;
    uint32_t r = count;

    /* when num is higher than 1/2 of UINT32_MAX the we just move slower
        (just some boundary not that it could actually happen with OS env) (its stupid i know) */ 
    if(*num >= hashtable_prime_capacities[count - 1]) {
        if(*num >= CLOSEST_UMAX32_PRIME) {
            return ERROR_INT_OVERFLOW;
        }

        // diffrent types of resizes
        uint32_t prime_lookup_start;
        if(*num >= CLOSEST_UMAX32_PRIME / 2) {
            prime_lookup_start = *num + 1;
        }
        else {
            prime_lookup_start = *num * 2;
        }
        *num = closestHigherPrime(prime_lookup_start);
        return SUCCESS;
    }

    // binary search
    while(l < r) {
        uint32_t middle = l + (r - l) / 2;
    
        if (*num >= hashtable_prime_capacities[middle]) {
            l = middle + 1;
        }
        else {
            r = middle;
        }
    } // while

    // if the prime is in the array return next index otherwise return next index
    if (l < count) {
        *num = hashtable_prime_capacities[l];
        return SUCCESS;
    }

    if (*num > UINT32_MAX / 2) {
        return ERROR_INT_OVERFLOW;
    }

    *num = closestHigherPrime(*num * 2);
    return SUCCESS;

} // hashTableNextPrime


/**
 * @brief  Finds the smallest prime number greater than num
 *
 * @param  num  Starting value for the prime search
 * @return The closest prime number > num
 */
static uint32_t closestHigherPrime(uint32_t num) {
    if (num <= 2) return 2;
    if (num % 2 == 0) num++;

    while (!isPrime(num)) {
        num += 2;
    }
    return num;
} // closestHigherPrime


/**
 * @brief  Tests whether a number is prime
 *
 * @param  n  Number to test
 * @return 1 if n is prime, 0 otherwise
 */
static uint8_t isPrime(uint32_t n) {
    if (n < 2) return 0U;
    if (n == 2 || n == 3) return 1U;
    if (n % 2 == 0 || n % 3 == 0) return 0U;

    for (uint32_t i = 5; i <= n / i; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0){
            return 0U;
        }
    }
    return 1U;
} // isPrime