# LOCiD C Library API Documentation

This document provides documentation for the C library API of the LOCiD tool.

## Header File

To use the library, include the `locid.h` header file:
```c
#include "locid.h"
```

## Data Structures

### `LOCiD_Result`
A struct that holds the result of a `LOCiD_Generate` operation.

**Fields:**
- `char* id_string`: A heap-allocated string containing the generated LOCiD. The caller is responsible for freeing this string using `LOCiD_FreeResult`.
- `int error_code`: An error code indicating the status of the operation. `0` on success, `3` on permission error, `5` on partial success.

### `LOCiD_Locus`
A struct that represents a single locus.

**Fields:**
- `char* key`: The name of the locus.
- `char* value`: The value of the locus. `_NA_` if the locus is not available.

### `LOCiD_LociList`
A struct that holds a list of loci.

**Fields:**
- `LOCiD_Locus* loci`: A pointer to an array of `LOCiD_Locus` structs.
- `int loci_count`: The number of loci in the array.
- `int error_code`: An error code indicating the status of the operation.

## Functions

### `LOCiD_Result LOCiD_Generate(const char* device_path, int chars, int volatility, const char* loci, char** stability_rank, int* na_count)`
Generates the LOCiD for a specified device.

**Parameters:**
- `device_path`: The path to the device (e.g., `/dev/sda`).
- `chars`: The desired length of the output hash string (1-256).
- `volatility`: The volatility level (0 for Hardware + Logic, 1 for Hardware Only).
- `loci`: A comma-separated string of locus numbers to include in the hash. Overrides `volatility`.
- `stability_rank`: An optional output parameter. If not `NULL`, it will be populated with a string representing the stability rank ("High", "Medium", or "Low"). The caller is responsible for freeing this string.
- `na_count`: An optional output parameter. If not `NULL`, it will be populated with the number of unavailable loci.

**Returns:**
A `LOCiD_Result` struct containing the generated LOCiD and an error code.

### `LOCiD_LociList LOCiD_List(const char* device_path, int volatility, const char* loci)`
Lists the loci and their values for a specified device.

**Parameters:**
- `device_path`: The path to the device (e.g., `/dev/sda`).
- `volatility`: The volatility level.
- `loci`: A comma-separated string of locus numbers to include in the list.

**Returns:**
A `LOCiD_LociList` struct containing the list of loci and an error code.

### `void LOCiD_FreeResult(LOCiD_Result* result)`
Frees the memory allocated for a `LOCiD_Result` struct.

**Parameters:**
- `result`: A pointer to the `LOCiD_Result` struct to be freed.

### `void LOCiD_FreeLociList(LOCiD_LociList* list)`
Frees the memory allocated for a `LOCiD_LociList` struct.

**Parameters:**
- `list`: A pointer to the `LOCiD_LociList` struct to be freed.
