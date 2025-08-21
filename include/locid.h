#ifndef LOCID_H
#define LOCID_H

#define LOCID_ERR_SUCCESS 0
#define LOCID_ERR_PARTIAL 5
#define LOCID_ERR_PERMISSION 3

// Result struct for generating a LOCiD
typedef struct {
    char* id_string;           // Heap-allocated LOCiD. Must be freed.
    int error_code;            // 0 on success, 5 on partial, 3 on permission error.
} LOCiD_Result;

// Result struct for listing loci
typedef struct {
    char* key;                 // Locus name
    char* value;               // Locus value (_NA_ on failure)
} LOCiD_Locus;

typedef struct {
    LOCiD_Locus* loci;         // Array of loci.
    int loci_count;            // Number of loci returned.
    int error_code;
} LOCiD_LociList;

/**
 * @brief Generates the LOCiD for a specified device.
 * @param device_path Path to the device.
 * @param chars Desired length (1-256).
 * @param volatility -1 for default (0), or 0/1 for presets (dynamic per type).
 * @param loci NULL or comma-separated locus numbers (overrides volatility).
 * @return LOCiD_Result. Free id_string. error_code 5 if partial metadata (message logged).
 */
LOCiD_Result LOCiD_Generate(const char* device_path, int chars, int volatility, const char* loci, char** stability_rank, int* na_count);

/**
 * @brief Lists loci and values.
 * @param device_path Path to the device.
 * @param volatility -1 for default, or 0/1 (dynamic).
 * @param loci NULL or comma-separated (overrides).
 * @return LOCiD_LociList. Free list and contents.
 */
LOCiD_LociList LOCiD_List(const char* device_path, int volatility, const char* loci);

/**
 * @brief Frees a LOCiD_Result.
 */
void LOCiD_FreeResult(LOCiD_Result* result);

/**
 * @brief Frees a LOCiD_LociList and its contents.
 */
void LOCiD_FreeLociList(LOCiD_LociList* list);

#endif // LOCID_H
