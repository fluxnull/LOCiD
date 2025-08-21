#ifndef HASHING_H
#define HASHING_H

#include <stddef.h>
#include <stdint.h>

// Hashes the input string using BLAKE3 and returns a hex-encoded hash of the specified length.
// The caller is responsible for freeing the returned string.
char* hash_string(const char* input, int output_len);

// Hashes the input buffer using BLAKE3 and returns a hex-encoded hash of the specified length.
// The caller is responsible for freeing the returned string.
char* hash_buffer(const void* input, size_t input_len, int output_len);

#endif // HASHING_H
