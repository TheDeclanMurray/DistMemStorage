#include "hashmap.h"
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

// Citation: https://www.algolist.net/Data_structures/Hash_table/Chaining, to remind
// how hash tables work

/**
 * Hash a string using sha1.
 *
 * @param value The value to be hashed
 * @param digest The array to store the hashed value.
 */
void hash(char *value, unsigned char *digest)
{
    size_t length = strlen(value);
    SHA1(value, length, digest);
}

/**
 * Set a new value associated with a key in the hashmap.
 * @param key The key associated with the value
 * @param val The value associated with the key
 * @param table The hashtable to perform the operation on
 *
 * @returns a boolean indicating if it was successful
 */
bool ht_set(char *key, char *val, HashTable_t *table)
{
    unsigned char digest[SHA_DIGEST_LENGTH];

    // Obtain 8 bytes that we can actually deal with
    hash(key, digest);
    uint64_t hashedint;
    memcpy(&hashedint, digest, 8);
    

    hashedint = hashedint % table->size;

    HT_Chain_t *cur = table->items[hashedint];

    // There is no items in the chain
    if (cur == NULL)
    {
        HT_Chain_t *new = malloc(sizeof(HT_Chain_t));
        new->key = strdup(key);
        new->value = strdup(val);
        new->next = NULL;
        table->items[hashedint] = new;
        table->count++; // We added a new item
    }
    else
    {
        // Check all the items and make sure our key isn't in use
        while (cur->next != NULL)
        {
            // If we have a matching key, we need to overwrite it's value and then we're done!
            if (strcmp(key, cur->key) == 0)
            {
                free(cur->value);
                cur->value = strdup(val);
                return true;
            }
            cur = cur->next;
        }
        if (strcmp(key, cur->key) == 0)
        {
            free(cur->value);
            cur->value = strdup(val);
            return true;
        }
        // We've reached the end of the chain and need to add onto it
        HT_Chain_t *new = malloc(sizeof(HT_Chain_t));
        new->key = strdup(key);
        new->value = strdup(val);
        new->next = NULL;
        cur->next = new;
    }
    /* If we have filled up over half the hashtable, we should expand it*/
    if (table->count > (table->size / 2))
    {
        ht_expand(table);
    }
    return true;
}

/**
 * Get a value associated with a key from the hashtable
 * @param key The key associated with the value we want
 * @param table A pointer to our hashtable
 * 
 * @returns an allocated string, representing the value, or NULL if no value was found
 */
char *ht_get(char *key, HashTable_t* table)
{
    unsigned char digest[SHA_DIGEST_LENGTH];
    char* ret;

    hash(key, digest);
    uint64_t hashedint;
    memcpy(&hashedint, digest, 8);
    

    hashedint = hashedint % table->size;

    HT_Chain_t *cur = table->items[hashedint];

    while (cur != NULL) {
        if (strcmp(key, cur->key) == 0) {
            ret = strdup(cur->value);
            return ret;
        } 
        cur = cur->next;      
    }
    return NULL;
}

bool ht_remove(char* key, HashTable_t* table) {

}


