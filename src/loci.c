#include "loci.h"
#include "hashing.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "globals.h"
#include <errno.h>
#include <stdbool.h>

#ifndef _WIN32
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <unistd.h>
#include <libgen.h>
#endif

void parse_loci_selection(const char* selection_str, bool* selection) {
    for (int i = 0; i < 19; i++) {
        selection[i] = false;
    }

    char* str = strdup(selection_str);
    char* token = strtok(str, ",");
    while (token != NULL) {
        char* dash = strchr(token, '-');
        if (dash) {
            // Range
            *dash = '\0';
            int start = atoi(token);
            int end = atoi(dash + 1);
            for (int i = start; i <= end; i++) {
                if (i >= 1 && i <= 18) {
                    selection[i] = true;
                }
            }
        } else {
            // Single number
            int num = atoi(token);
            if (num >= 1 && num <= 18) {
                selection[num] = true;
            }
        }
        token = strtok(NULL, ",");
    }
    free(str);
}

#ifndef _WIN32
// Helper function to read a file from sysfs.
// This is used to read simple string values from files in /sys/block/<device>/device/
static char* read_sysfs_file(const char* device_name, const char* file) {
    char path[256];
    sprintf(path, "/sys/block/%s/device/%s", device_name, file);
    FILE* f = fopen(path, "r");
    if (f) {
        char* line = NULL;
        size_t len = 0;
        getline(&line, &len, f);
        fclose(f);
        // Remove trailing newline
        if (line && strlen(line) > 0 && line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        return line;
    }
    return strdup("_NA_");
}

// Helper function to get the total size of a device in bytes
static unsigned long long get_total_size(const char* device_path) {
    int fd = open(device_path, O_RDONLY);
    if (fd >= 0) {
        unsigned long long size_in_bytes;
        if (ioctl(fd, BLKGETSIZE64, &size_in_bytes) == 0) {
            close(fd);
            return size_in_bytes;
        }
        close(fd);
    }
    return 0;
}

// Helper function to get the sector size of a device in bytes
static int get_sector_size(const char* device_path) {
    int fd = open(device_path, O_RDONLY);
    if (fd >= 0) {
        int sector_size;
        if (ioctl(fd, BLKSSZGET, &sector_size) == 0) {
            close(fd);
            return sector_size;
        }
        close(fd);
    }
    return 0;
}

// Helper function to read a file from the queue directory in sysfs.
// This is used to read simple string values from files in /sys/block/<device>/queue/
static char* read_sysfs_queue_file(const char* device_name, const char* file) {
    char path[256];
    sprintf(path, "/sys/block/%s/queue/%s", device_name, file);
    FILE* f = fopen(path, "r");
    if (f) {
        char* line = NULL;
        size_t len = 0;
        getline(&line, &len, f);
        fclose(f);
        // Remove trailing newline
        if (line && strlen(line) > 0 && line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        return line;
    }
    return strdup("_NA_");
}
#endif

int collect_loci(const char* device_path, Loci* loci) {
#ifndef _WIN32
    char* device_path_copy = strdup(device_path);
    char* device_name = basename(device_path_copy);

    // Get Model and SerialNumber from sysfs
    loci->model = read_sysfs_file(device_name, "model");
    if (verbose_output) fprintf(stderr, "Model: %s\n", loci->model);
    loci->serial_number = read_sysfs_file(device_name, "serial");
    if (verbose_output) fprintf(stderr, "SerialNumber: %s\n", loci->serial_number);

    // Get RotationRate from sysfs
    loci->rotation_rate = read_sysfs_queue_file(device_name, "rotational");
    if (verbose_output) fprintf(stderr, "RotationRate: %s\n", loci->rotation_rate);

    // Get FirmwareRevision from sysfs
    loci->firmware_revision = read_sysfs_file(device_name, "firmware_rev");
    if (verbose_output) fprintf(stderr, "FirmwareRevision: %s\n", loci->firmware_revision);

    free(device_path_copy);

    unsigned long long total_size = get_total_size(device_path);
    if (total_size > 0) {
        loci->total_size = malloc(21);
        sprintf(loci->total_size, "%llu", total_size);
    } else {
        loci->total_size = strdup("_NA_");
    }
    if (verbose_output) fprintf(stderr, "TotalSize: %s\n", loci->total_size);

    int sector_size = get_sector_size(device_path);
    if (sector_size > 0) {
        loci->sector_sizes = malloc(12);
        sprintf(loci->sector_sizes, "%d", sector_size);
    } else {
        loci->sector_sizes = strdup("_NA_");
    }
    if (verbose_output) fprintf(stderr, "SectorSizes: %s\n", loci->sector_sizes);

    // Get LastSectorHash
    if (total_size > 0 && sector_size > 0) {
        int fd_lsh = open(device_path, O_RDONLY);
        if (fd_lsh >= 0) {
            char* buffer = malloc(sector_size);
            off_t offset = total_size - sector_size;
            ssize_t read_ret = -1;
            if (lseek(fd_lsh, offset, SEEK_SET) != -1) {
                read_ret = read(fd_lsh, buffer, sector_size);
            }
            close(fd_lsh);

            if (read_ret == sector_size) {
                loci->last_sector_hash = hash_buffer(buffer, sector_size, 64);
            } else {
                loci->last_sector_hash = strdup("_NA_");
            }

            free(buffer);
        } else {
            loci->last_sector_hash = strdup("_NA_");
        }
    } else {
        loci->last_sector_hash = strdup("_NA_");
    }
    if (verbose_output) fprintf(stderr, "LastSectorHash: %s\n", loci->last_sector_hash);

    // Get DiskGUID by reading the GPT header.
    // The GPT header is located at LBA 1 (512 bytes offset).
    // The Disk GUID is a 16-byte field at offset 56 of the header.
    int fd_guid = open(device_path, O_RDONLY);
    if (fd_guid >= 0) {
        char gpt_header[92];
        lseek(fd_guid, 512, SEEK_SET);
        read(fd_guid, gpt_header, 92);
        close(fd_guid);

        // Check for the "EFI PART" signature at the beginning of the header.
        if (memcmp(gpt_header, "EFI PART", 8) == 0) {
            unsigned char* guid = (unsigned char*)gpt_header + 56;
            char* guid_str = malloc(37);
            sprintf(guid_str, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                    guid[3], guid[2], guid[1], guid[0],
                    guid[5], guid[4],
                    guid[7], guid[6],
                    guid[8], guid[9],
                    guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]);
            loci->disk_guid = guid_str;
        } else {
            loci->disk_guid = strdup("_NA_");
        }
    } else {
        loci->disk_guid = strdup("_NA_");
    }
    if (verbose_output) fprintf(stderr, "DiskGUID: %s\n", loci->disk_guid);


    // Stub out the rest of the loci for now
    loci->world_wide_name = strdup("_NA_");
    loci->controller_id = strdup("_NA_");
    loci->disk_shift = strdup("_NA_");
    loci->emmc_cid = strdup("_NA_");
    loci->ufs_uid = strdup("_NA_");
    loci->raid_controller_model = strdup("_NA_");
    loci->raid_volume_uid = strdup("_NA_");
    loci->standards_version = strdup("_NA_");
    loci->supported_features_hash = strdup("_NA_");
    loci->raid_level = strdup("_NA_");

    return 0;

#else // Stubs for Windows
    loci->serial_number = strdup("_NA_");
    loci->world_wide_name = strdup("_NA_");
    loci->model = strdup("_NA_");
    loci->controller_id = strdup("_NA_");
    loci->disk_shift = strdup("_NA_");
    loci->emmc_cid = strdup("_NA_");
    loci->ufs_uid = strdup("_NA_");
    loci->raid_controller_model = strdup("_NA_");
    loci->raid_volume_uid = strdup("_NA_");
    loci->total_size = strdup("_NA_");
    loci->rotation_rate = strdup("_NA_");
    loci->sector_sizes = strdup("_NA_");
    loci->firmware_revision = strdup("_NA_");
    loci->standards_version = strdup("_NA_");
    loci->supported_features_hash = strdup("_NA_");
    loci->raid_level = strdup("_NA_");
    loci->disk_guid = strdup("_NA_");
    loci->last_sector_hash = strdup("_NA_");
    return 0;
#endif
}

void combine_loci(const Loci* loci, char* output, int volatility, const char* loci_selection) {
    strcpy(output, "");

    if (loci_selection) {
        // Expert mode: use the --loci selection
        bool selection[19];
        parse_loci_selection(loci_selection, selection);

        if (selection[1]) sprintf(output + strlen(output), "SerialNumber:%s\n", loci->serial_number);
        if (selection[2]) sprintf(output + strlen(output), "WorldWideName:%s\n", loci->world_wide_name);
        if (selection[3]) sprintf(output + strlen(output), "Model:%s\n", loci->model);
        if (selection[4]) sprintf(output + strlen(output), "ControllerID:%s\n", loci->controller_id);
        if (selection[5]) sprintf(output + strlen(output), "DiskShift:%s\n", loci->disk_shift);
        if (selection[6]) sprintf(output + strlen(output), "eMMC_CID:%s\n", loci->emmc_cid);
        if (selection[7]) sprintf(output + strlen(output), "UFS_UID:%s\n", loci->ufs_uid);
        if (selection[8]) sprintf(output + strlen(output), "RAID_Controller_Model:%s\n", loci->raid_controller_model);
        if (selection[9]) sprintf(output + strlen(output), "RAID_Volume_UID:%s\n", loci->raid_volume_uid);
        if (selection[10]) sprintf(output + strlen(output), "TotalSize:%s\n", loci->total_size);
        if (selection[11]) sprintf(output + strlen(output), "RotationRate:%s\n", loci->rotation_rate);
        if (selection[12]) sprintf(output + strlen(output), "SectorSizes:%s\n", loci->sector_sizes);
        if (selection[13]) sprintf(output + strlen(output), "FirmwareRevision:%s\n", loci->firmware_revision);
        if (selection[14]) sprintf(output + strlen(output), "StandardsVersion:%s\n", loci->standards_version);
        if (selection[15]) sprintf(output + strlen(output), "SupportedFeaturesHash:%s\n", loci->supported_features_hash);
        if (selection[16]) sprintf(output + strlen(output), "RAID_Level:%s\n", loci->raid_level);
        if (selection[17]) sprintf(output + strlen(output), "DiskGUID:%s\n", loci->disk_guid);
        if (selection[18]) sprintf(output + strlen(output), "LastSectorHash:%s\n", loci->last_sector_hash);
    } else {
        // Preset mode: use the --volatility level
        if (volatility == 0) {
            // Hardware + Logic
            sprintf(output + strlen(output), "SerialNumber:%s\n", loci->serial_number);
            sprintf(output + strlen(output), "WorldWideName:%s\n", loci->world_wide_name);
            sprintf(output + strlen(output), "Model:%s\n", loci->model);
            sprintf(output + strlen(output), "ControllerID:%s\n", loci->controller_id);
            sprintf(output + strlen(output), "DiskShift:%s\n", loci->disk_shift);
            sprintf(output + strlen(output), "eMMC_CID:%s\n", loci->emmc_cid);
            sprintf(output + strlen(output), "UFS_UID:%s\n", loci->ufs_uid);
            sprintf(output + strlen(output), "RAID_Controller_Model:%s\n", loci->raid_controller_model);
            sprintf(output + strlen(output), "RAID_Volume_UID:%s\n", loci->raid_volume_uid);
            sprintf(output + strlen(output), "TotalSize:%s\n", loci->total_size);
            sprintf(output + strlen(output), "RotationRate:%s\n", loci->rotation_rate);
            sprintf(output + strlen(output), "SectorSizes:%s\n", loci->sector_sizes);
            sprintf(output + strlen(output), "FirmwareRevision:%s\n", loci->firmware_revision);
            sprintf(output + strlen(output), "StandardsVersion:%s\n", loci->standards_version);
            sprintf(output + strlen(output), "SupportedFeaturesHash:%s\n", loci->supported_features_hash);
            sprintf(output + strlen(output), "RAID_Level:%s\n", loci->raid_level);
            sprintf(output + strlen(output), "DiskGUID:%s\n", loci->disk_guid);
            sprintf(output + strlen(output), "LastSectorHash:%s\n", loci->last_sector_hash);
        } else if (volatility == 1) {
            // Hardware Only
            sprintf(output + strlen(output), "SerialNumber:%s\n", loci->serial_number);
            sprintf(output + strlen(output), "WorldWideName:%s\n", loci->world_wide_name);
            sprintf(output + strlen(output), "Model:%s\n", loci->model);
            sprintf(output + strlen(output), "ControllerID:%s\n", loci->controller_id);
            sprintf(output + strlen(output), "DiskShift:%s\n", loci->disk_shift);
            sprintf(output + strlen(output), "eMMC_CID:%s\n", loci->emmc_cid);
            sprintf(output + strlen(output), "UFS_UID:%s\n", loci->ufs_uid);
            sprintf(output + strlen(output), "RAID_Controller_Model:%s\n", loci->raid_controller_model);
            sprintf(output + strlen(output), "RAID_Volume_UID:%s\n", loci->raid_volume_uid);
            sprintf(output + strlen(output), "TotalSize:%s\n", loci->total_size);
            sprintf(output + strlen(output), "RotationRate:%s\n", loci->rotation_rate);
            sprintf(output + strlen(output), "SectorSizes:%s\n", loci->sector_sizes);
            sprintf(output + strlen(output), "FirmwareRevision:%s\n", loci->firmware_revision);
            sprintf(output + strlen(output), "StandardsVersion:%s\n", loci->standards_version);
            sprintf(output + strlen(output), "SupportedFeaturesHash:%s\n", loci->supported_features_hash);
            sprintf(output + strlen(output), "RAID_Level:%s\n", loci->raid_level);
        }
    }
}
