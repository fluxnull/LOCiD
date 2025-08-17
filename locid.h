#ifndef LOCID_H
#define LOCID_H

#include <stdlib.h>
#include <stdint.h>
#include "blake3.h"  // BLAKE3 reference

#define MAX_LOCI 18
#define HASH_SIZE 128  // 1024 bits = 128 bytes

typedef struct {
    char* key;
    char* value;
} LOCiD_Locus;

typedef struct {
    LOCiD_Locus* loci;
    int loci_count;
    int error_code;
} LOCiD_LociList;

typedef struct {
    char* id_string;
    int error_code;
} LOCiD_Result;

// Main generate function
LOCiD_Result LOCiD_Generate(const char* device_path, int chars, int volatility, const char* loci_str);

// List loci
LOCiD_LociList LOCiD_List(const char* device_path, int volatility, const char* loci_str);

// Free results
void LOCiD_FreeResult(LOCiD_Result* result);
void LOCiD_FreeLociList(LOCiD_LociList* list);

#endif // LOCID_H