#include "locid.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#include <winioctl.h>
#include <ntddstor.h>
#include <ntddvol.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <linux/hdreg.h>
#include <scsi/sg.h>
#include <scsi/scsi.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

// Loci definitions
typedef struct {
    int num;
    const char* name;
    const char* category;
    double change_prob;  // From matrix
} LocusInfo;

static const LocusInfo loci_info[MAX_LOCI] = {
    {1, "SerialNumber", "Immutable", 0.00001},
    {2, "World Wide Name (WWN)", "Immutable", 0.00001},
    {3, "Model", "Immutable", 0.00001},
    {4, "ControllerID (NVMe)", "Immutable", 0.00001},
    {5, "DiskShift (SMART 220)", "Immutable", 0.0001},
    {6, "eMMC_CID", "Immutable", 0.00001},
    {7, "UFS_UID", "Immutable", 0.00001},
    {8, "RAID_Controller_Model", "Immutable", 0.00001},
    {9, "RAID_Volume_UID", "Immutable", 0.00001},
    {10, "TotalSize", "Immutable", 0.0001},
    {11, "RotationRate", "Immutable", 0.0001},
    {12, "SectorSizes", "Immutable", 0.0001},
    {13, "FirmwareRevision", "Stateful", 0.0001},
    {14, "StandardsVersion", "Stateful", 0.0001},
    {15, "SupportedFeaturesHash", "Stateful", 0.0001},
    {16, "RAID_Level", "Stateful", 0.0001},
    {17, "DiskGUID", "Logical", 0.05},
    {18, "LastSectorHash", "Logical", 0.05}
};

// Device type enum for dynamic filtering
typedef enum {
    TYPE_UNKNOWN,
    TYPE_HDD,
    TYPE_SSD,
    TYPE_NVME,
    TYPE_RAID,
    TYPE_EMMC,
    TYPE_UFS,
    TYPE_USB
} DeviceType;

// Function prototypes (internal)
static DeviceType detect_device_type(const char* device_path);
static int* parse_loci(const char* loci_str, int* count);
static int collect_loci(const char* device_path, DeviceType type, int volatility, int* selected, int selected_count, LOCiD_Locus** loci_out, int* count_out);
static char* compute_hash(const LOCiD_Locus* loci, int count, int chars);
static char* compute_stability_rank(const LOCiD_Locus* loci, int count);
static void log_verbose(const char* msg);
static void free_loci(LOCiD_Locus* loci, int count);

// Detect device type
static DeviceType detect_device_type(const char* device_path) {
    // Impl: Use Model, RotationRate, etc. via queries
    // For example:
    // If RotationRate >0, HDD; =0 SSD; NVMe via controller; RAID via API; eMMC/UFS via CID/UID; USB via enclosure block.
    // Stub: Return TYPE_SSD for demo
    return TYPE_SSD;
}

// Parse --loci "1,3-5,10"
static int* parse_loci(const char* loci_str, int* count) {
    if (!loci_str) {
        *count = 0;
        return NULL;
    }
    // Parse comma, ranges
    int* selected = malloc(MAX_LOCI * sizeof(int));
    *count = 0;
    char* copy = strdup(loci_str);
    char* token = strtok(copy, ",");
    while (token) {
        if (strchr(token, '-')) {
            int start = atoi(token);
            int end = atoi(strchr(token, '-') + 1);
            for (int i = start; i <= end; i++) {
                selected[(*count)++] = i;
            }
        } else {
            selected[(*count)++] = atoi(token);
        }
        token = strtok(NULL, ",");
    }
    free(copy);
    return selected;
}

// Collect loci based on volatility, selected, type
static int collect_loci(const char* device_path, DeviceType type, int volatility, int* selected, int selected_count, LOCiD_Locus** loci_out, int* count_out) {
    LOCiD_Locus* loci = malloc(MAX_LOCI * sizeof(LOCiD_Locus));
    int count = 0;
    int partial = 0;

    for (int i = 0; i < MAX_LOCI; i++) {
        int num = loci_info[i].num;
        if (volatility == 1 && strcmp(loci_info[i].category, "Logical") == 0) continue;
        if (selected_count > 0) {
            int match = 0;
            for (int j = 0; j < selected_count; j++) if (selected[j] == num) { match = 1; break; }
            if (!match) continue;
        }
        // Dynamic filter: Skip if irrelevant to type (e.g., DiskShift for non-HDD)
        // Impl stub: Assume all applicable
        // Query value
        char* value = malloc(256);
        // Stub query: sprintf(value, "Value for %s", loci_info[i].name);
        // If fail: strcpy(value, "_NA_"); partial = 1;
        strcpy(value, "StubValue");  // Replace with actual queries
        if (strcmp(value, "_NA_") == 0) partial = 1;
        loci[count].key = strdup(loci_info[i].name);
        loci[count].value = value;
        count++;
    }

    *loci_out = loci;
    *count_out = count;
    return partial;
}

// Compute BLAKE3 hash from loci
static char* compute_hash(const LOCiD_Locus* loci, int count, int chars) {
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    for (int i = 0; i < count; i++) {
        blake3_hasher_update(&hasher, loci[i].key, strlen(loci[i].key));
        blake3_hasher_update(&hasher, ":", 1);
        blake3_hasher_update(&hasher, loci[i].value, strlen(loci[i].value));
        blake3_hasher_update(&hasher, "\n", 1);
    }
    uint8_t output[HASH_SIZE];
    blake3_hasher_finalize_seek(&hasher, 0, output, HASH_SIZE);  // Extended
    char* hex = malloc(HASH_SIZE * 2 + 1);
    for (int i = 0; i < HASH_SIZE; i++) sprintf(hex + i*2, "%02x", output[i]);
    hex[chars * 2] = '\0';  // Truncate
    return hex;
}

// Compute stability rank
static char* compute_stability_rank(const LOCiD_Locus* loci, int count) {
    double score = 0.0;
    int non_na = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(loci[i].value, "_NA_") != 0) {
            non_na++;
            // Find change_prob
            for (int j = 0; j < MAX_LOCI; j++) {
                if (strcmp(loci[i].key, loci_info[j].name) == 0) {
                    score += 1.0 - loci_info[j].change_prob;
                    break;
                }
            }
        }
    }
    double avg_score = count > 0 ? score / count : 0.0;
    double avail_ratio = (double)non_na / count;
    double final = avg_score * avail_ratio;
    if (final > 0.8) return "High";
    if (final > 0.5) return "Medium";
    return "Low";
}

// Verbose log
static void log_verbose(const char* msg) {
    fprintf(stderr, "%s\n", msg);
}

// Free loci
static void free_loci(LOCiD_Locus* loci, int count) {
    for (int i = 0; i < count; i++) {
        free(loci[i].key);
        free(loci[i].value);
    }
    free(loci);
}

// Library Generate
LOCiD_Result LOCiD_Generate(const char* device_path, int chars, int volatility, const char* loci_str) {
    LOCiD_Result result = {NULL, 0};
    DeviceType type = detect_device_type(device_path);
    int* selected = NULL;
    int selected_count = 0;
    if (loci_str) selected = parse_loci(loci_str, &selected_count);
    LOCiD_Locus* loci;
    int count;
    int partial = collect_loci(device_path, type, volatility, selected, selected_count, &loci, &count);
    free(selected);
    if (count == 0) {
        result.error_code = 4;
        return result;
    }
    result.id_string = compute_hash(loci, count, chars);
    char* rank = compute_stability_rank(loci, count);
    if (partial) {
        fprintf(stderr, "Only partial metadata was captured: we got a signature but it's missing field values.\n");
        result.error_code = 5;
    }
    // Append rank if --stability, but since library, assume caller handles
    // Stub: Not appended here
    free_loci(loci, count);
    free(rank);
    return result;
}

// Library List
LOCiD_LociList LOCiD_List(const char* device_path, int volatility, const char* loci_str) {
    LOCiD_LociList list = {NULL, 0, 0};
    DeviceType type = detect_device_type(device_path);
    int* selected = NULL;
    int selected_count = 0;
    if (loci_str) selected = parse_loci(loci_str, &selected_count);
    int partial = collect_loci(device_path, type, volatility, selected, selected_count, &list.loci, &list.loci_count);
    free(selected);
    if (partial) list.error_code = 5;
    return list;
}

// Free
void LOCiD_FreeResult(LOCiD_Result* result) {
    free(result->id_string);
}

void LOCiD_FreeLociList(LOCiD_LociList* list) {
    free_loci(list->loci, list->loci_count);
}

// Main executable
int main(int argc, char** argv) {
    // Parse args (stub: use getopt)
    const char* device_path = "default";
    int chars = 256;
    int volatility = 0;
    const char* loci_str = NULL;
    int verbose = 0;
    int stability = 0;
    int json = 0;
    int list = 0;

    // Stub parsing...
    // Assume parsed

    if (list) {
        LOCiD_LociList l = LOCiD_List(device_path, volatility, loci_str);
        // Print table or JSON
        if (json) {
            printf("{\"loci\":[");
            for (int i = 0; i < l.loci_count; i++) {
                printf("{\"key\":\"%s\",\"value\":\"%s\"}%s", l.loci[i].key, l.loci[i].value, i < l.loci_count - 1 ? "," : "");
            }
            printf("]}\n");
        } else {
            printf("# | Locus Name | Value\n");
            for (int i = 0; i < l.loci_count; i++) {
                printf("%d | %s | %s\n", i+1, l.loci[i].key, l.loci[i].value);
            }
        }
        LOCiD_FreeLociList(&l);
        return l.error_code;
    }

    LOCiD_Result r = LOCiD_Generate(device_path, chars, volatility, loci_str);
    if (r.id_string) {
        if (stability) {
            // Calc rank from internal (re-collect or cache)
            // Stub: printf("%s [Stability: High]\n", r.id_string);
        } else {
            printf("%s\n", r.id_string);
        }
    }
    LOCiD_FreeResult(&r);
    return r.error_code;
}