#pragma once
#include <stdbool.h>

typedef struct HT_Chain {
    char* key;
    char* value;
    struct HT_Chain* next; // Pointer to the next item in case of collision
} HT_Chain_t;

typedef struct HashTable {
    HT_Chain_t** items;     // Array of pointers to Ht_item
    int size;            // Total number of buckets
    int count;           // Current number of items
} HashTable_t;

void hash(char* value, unsigned char* digest);

bool ht_set(char* key, char* val, HashTable_t* table);

char* ht_get(char* key, HashTable_t* table);

bool ht_remove(char* key);

bool ht_expand(HashTable_t* t);