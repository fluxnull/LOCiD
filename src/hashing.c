#include "hashing.h"
#include "blake3.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define HASH_BYTE_SIZE 128

char* hash_string(const char* input, int output_len) {
    uint8_t hash[HASH_BYTE_SIZE];
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, input, strlen(input));
    blake3_hasher_finalize_seek(&hasher, 0, hash, HASH_BYTE_SIZE);

    char* hex_hash = malloc(HASH_BYTE_SIZE * 2 + 1);
    for (size_t i = 0; i < HASH_BYTE_SIZE; i++) {
        sprintf(hex_hash + 2 * i, "%02x", hash[i]);
    }

    if (output_len > 0 && output_len < HASH_BYTE_SIZE * 2) {
        hex_hash[output_len] = '\0';
    }

    return hex_hash;
}

char* hash_buffer(const void* input, size_t input_len, int output_len) {
    uint8_t hash[HASH_BYTE_SIZE];
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, input, input_len);
    blake3_hasher_finalize_seek(&hasher, 0, hash, HASH_BYTE_SIZE);

    char* hex_hash = malloc(HASH_BYTE_SIZE * 2 + 1);
    for (size_t i = 0; i < HASH_BYTE_SIZE; i++) {
        sprintf(hex_hash + 2 * i, "%02x", hash[i]);
    }

    if (output_len > 0 && output_len < HASH_BYTE_SIZE * 2) {
        hex_hash[output_len] = '\0';
    }

    return hex_hash;
}
