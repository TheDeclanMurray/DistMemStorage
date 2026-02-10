#include "hashmap.h"
#include <ctype.h>

/**
 * Hash a string using sha1.
 */
unsigned char hash(char* value) {
    size_t length = strlen(value);
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(data, length, hash);
}

