#ifndef LOCI_H
#define LOCI_H

#include <stdbool.h>

// A struct to hold all the collected loci
typedef struct {
    char* serial_number;
    char* world_wide_name;
    char* model;
    char* controller_id;
    char* disk_shift;
    char* emmc_cid;
    char* ufs_uid;
    char* raid_controller_model;
    char* raid_volume_uid;
    char* total_size;
    char* rotation_rate;
    char* sector_sizes;
    char* firmware_revision;
    char* standards_version;
    char* supported_features_hash;
    char* raid_level;
    char* disk_guid;
    char* last_sector_hash;
} Loci;

// Collects all the loci for a given device.
// Returns an error code (0 on success).
int collect_loci(const char* device_path, Loci* loci_data);

// Parses the loci selection string (e.g., "1,3-5,10") into a boolean array.
void parse_loci_selection(const char* selection_str, bool* selection);

// Combines the collected loci into a single string for hashing
// The output buffer must be large enough to hold the combined string.
void combine_loci(const Loci* loci, char* output, int volatility, const char* loci_selection);

#endif // LOCI_H
