/**
 * @file        htab.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   
 */

#include "data-structures/hash-table.h"

/** @brief Hash function 1
 *  @param key Pointer to the key string
 *  @return Hash value
 */
static uint32_t hash1(const char *key);


/** @brief Hash function 2
 *  @param key Pointer to the key string
 *  @return Hash value
 */
static uint32_t hash2(const char *key);

/** @brief Finds the index of a key in the hash table
 *  @param table Pointer to the hash table
 *  @param key Pointer to the key string
 *  @return Index of the key if found, -1 otherwise
 */
static int32_t hashTableFindIndex(HashTablePtr table, const char* key);

/** @brief Finds the next prime number greater than or equal to the given number
 *  @param num Pointer to the number
 *  @return SUCCESS if a prime is found, ERROR otherwise
 */
static StatusEnum hashTableNextPrime(uint32_t* num);

/** @brief Finds the closest higher prime number to the given number
 *  @param num The number
 *  @return The closest higher prime number
 */
static uint32_t closestHigherPrime(uint32_t num);

/** @brief Checks if a number is prime
 *  @param n The number to check
 *  @return 1 if the number is prime, 0 otherwise
 */
static uint8_t isPrime(uint32_t n);

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


static uint32_t hash1(const char *key) {
    uint32_t h = 2166136261U;

    while (*key) {
        h ^= (uint8_t)*key++;
        h *= 16777619U;
    }

    return h;
} // hash1



static uint32_t hash2(const char *key) {
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
        printError("hashTableCtor", "Malloc failure");
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



void hashTableDtor(HashTablePtr table) {
    if(table == NULL || table->data == NULL) {
        return;
    }

    // loop through whole hashtable and free occupied indexes
    // key and value share one allocation (value points inside the key block)
    for(uint32_t i = 0; i < table->capacity; i++) {
        HashTableItemPtr item = &(table->data[i]);
        if(item->state == ITEM_STATE_FULL) {
            free(item->key);
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



StatusEnum hashTableInsert(HashTablePtr table, const char* key, const char* value) {

    if(table == NULL || table->data == NULL || key == NULL || value == NULL) {
        printError("hashTableInsert", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }

    /* if hashtable is overloaded ( higher load than 0.675) 
        increase size to closest higher prime number
    */

    if((table->currentSize + 1) * LOAD_FACTOR_DENUM >= table->capacity * LOAD_FACTOR_NUM) {
        StatusEnum st = hashTableResize(table);
        ERR_CHECK(st);
    }

    int32_t index = hashTableFindIndex(table, key);
    if(index == -1) {
        return ERROR_INDEX_OUT_OF_BOUNDS;
    }

    HashTableItemPtr item = &(table->data[index]);

    /* allocate block on heap, `+ 2` for two \0 characters, one for key one for value */
    uint32_t key_length = strlen(key);
    uint32_t value_length = strlen(value); 
    char* block = (char*) malloc(key_length + value_length + 2);

    if (block == NULL) {
        printError("hashTableInsert", "Malloc failure");
        return ERROR_MALLOC_FAILURE;
    }

    // copy key into allocated block
    memcpy(block, key, key_length);
    block[key_length] = '\0';
    // copy value with '\0'
    memcpy(block + key_length + 1, value, value_length + 1);
    
    if(item->state == ITEM_STATE_FULL) {
        free(item->key);
    }
    else {
        table->currentSize++;
    }

    // move pointers
    item->key = block;
    item->value = block + key_length + 1;
    item->state = ITEM_STATE_FULL;
    return SUCCESS;
} // hashTableInsert



StatusEnum hashTableResize(HashTablePtr table) {
    uint32_t old_capacity = table->capacity;

    StatusEnum st = hashTableNextPrime(&(table->capacity));
    ERR_CHECK(st);

    HashTableItemPtr new_data = malloc(sizeof(HashTableItem) * table->capacity);
    if(new_data == NULL) {
        table->capacity = old_capacity;
        printError("hashTableResize", "Malloc failure");
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

    int32_t new_table_index;
    for(uint32_t i = 0; i < old_capacity; i++) {
        if(old_data[i].state == ITEM_STATE_FULL) {
            new_table_index = hashTableFindIndex(table, old_data[i].key);

            // indexing error
            if(new_table_index < 0) {
                free(new_data);
                table->data = old_data;
                table->capacity = old_capacity;
                printError("hashTableInsert", "Indexing failure");
                return ERROR_INDEX_OUT_OF_BOUNDS;
            }
            // move pointers to correct positions
            table->data[new_table_index].state = ITEM_STATE_FULL;
            table->data[new_table_index].key = old_data[i].key;
            table->data[new_table_index].value = old_data[i].value;
        } // if
    } // for

    free(old_data);
    return SUCCESS;
} // hashTableResize



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



StatusEnum hashTableRemove(HashTablePtr table, const char* key) {
    if(table == NULL || key == NULL || table->data == NULL) {
        printError("hashTableRemove", "Passing NULL pointer");
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
    // free one block
    if(item->key != NULL) {
        free(item->key);
    }

    item->key = NULL;
    item->value = NULL;
    table->currentSize--;
    item->state = ITEM_STATE_DELETED;
    return SUCCESS;
} // hashTableRemove 



static StatusEnum hashTableNextPrime(uint32_t* num) {
    if(num == NULL) {
        printError("hashTableNextPrime", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }
    uint32_t count = sizeof(hashtable_prime_capacities) / sizeof(hashtable_prime_capacities[0]);
    uint32_t l = 0;
    uint32_t r = count;

    /* when num is higher than 1/2 of UINT32_MAX the we just move slower
        (just some boundary not that it could actually happen with OS env) (its stupid i know) */ 
    if(*num >= hashtable_prime_capacities[count - 1]) {
        if(*num >= CLOSEST_UMAX32_PRIME) {
            printError("hashTableNextPrime", "Resize number too large for prime number lookup (int overflow)");
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
        printError("hashTableNextPrime", "Resize number too large for prime number lookup (int overflow)");
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


StatusEnum hashTableGetValue(HashTablePtr table, char* key, char** value) {
    if(key == NULL || table == NULL || table->data == NULL) {
        printError("hashTableGetValue", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }
    int32_t index = hashTableFindIndex(table, key);
    if(index < 0) {
        return ERROR_INDEX_OUT_OF_BOUNDS;
    }

    
    HashTableItemPtr item = &(table->data[index]);
    if(item->state == ITEM_STATE_EMPTY || item->state == ITEM_STATE_DELETED) {
        return ERROR_HTAB_ITEM;
    }
    // returning the value
    *value = item->value;
    return SUCCESS;
} // hashTableGetValue


uint32_t hashTableGetCurrSize(HashTablePtr table) {
    return table->currentSize;
}


void hashTableIterCtor(HashTableIterPtr table_iterator, HashTablePtr table) {
    table_iterator->table = table;
    table_iterator->index = 0;
}


uint8_t hashTableIterNext(HashTableIterPtr table_iterator, char** key, char** value) {
    while(table_iterator->index < table_iterator->table->capacity) {
        if(table_iterator->table->data[table_iterator->index].state == ITEM_STATE_FULL) {
            *key = table_iterator->table->data[table_iterator->index].key;
            *value = table_iterator->table->data[table_iterator->index].value;
            table_iterator->index++;
            return 1U;

        }
        table_iterator->index++;
    }
    return 0U;
}