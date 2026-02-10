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

unsigned char hash(char* value);

bool set(char* key, char* val);

char* get(char* key);

bool remove(char* key);

bool expand()