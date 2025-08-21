#include "locid.h"
#include "loci.h"
#include "hashing.h"
#include "stability.h"
#include <stdlib.h>
#include <string.h>

LOCiD_Result LOCiD_Generate(const char* device_path, int chars, int volatility, const char* loci, char** stability_rank, int* na_count) {
    LOCiD_Result result;
    result.id_string = NULL;

    Loci loci_data;
    int err = collect_loci(device_path, &loci_data);
    if (err) {
        result.error_code = err;
        return result;
    }
    result.error_code = 0;

    if (stability_rank) {
        *stability_rank = compute_stability_rank(&loci_data);
    }

    if (na_count) {
        *na_count = 0;
        const char* loci_values[] = {
            loci_data.serial_number, loci_data.world_wide_name, loci_data.model,
            loci_data.controller_id, loci_data.disk_shift, loci_data.emmc_cid,
            loci_data.ufs_uid, loci_data.raid_controller_model, loci_data.raid_volume_uid,
            loci_data.total_size, loci_data.rotation_rate, loci_data.sector_sizes,
            loci_data.firmware_revision, loci_data.standards_version,
            loci_data.supported_features_hash, loci_data.raid_level,
            loci_data.disk_guid, loci_data.last_sector_hash,
        };
        for (int i = 0; i < 18; i++) {
            if (strcmp(loci_values[i], "_NA_") == 0) {
                (*na_count)++;
            }
        }
    }

    char combined_loci[2048];
    combine_loci(&loci_data, combined_loci, volatility, loci);

    result.id_string = hash_string(combined_loci, chars);

    // Free the allocated loci strings
    free(loci_data.serial_number);
    free(loci_data.world_wide_name);
    free(loci_data.model);
    free(loci_data.controller_id);
    free(loci_data.disk_shift);
    free(loci_data.emmc_cid);
    free(loci_data.ufs_uid);
    free(loci_data.raid_controller_model);
    free(loci_data.raid_volume_uid);
    free(loci_data.total_size);
    free(loci_data.rotation_rate);
    free(loci_data.sector_sizes);
    free(loci_data.firmware_revision);
    free(loci_data.standards_version);
    free(loci_data.supported_features_hash);
    free(loci_data.raid_level);
    free(loci_data.disk_guid);
    free(loci_data.last_sector_hash);

    return result;
}

LOCiD_LociList LOCiD_List(const char* device_path, int volatility, const char* loci) {
    LOCiD_LociList result;
    result.loci = NULL;

    Loci loci_data;
    int err = collect_loci(device_path, &loci_data);
    if (err) {
        result.error_code = err;
        return result;
    }
    result.error_code = 0;

    result.loci_count = 18;
    result.loci = malloc(sizeof(LOCiD_Locus) * result.loci_count);

    result.loci[0].key = strdup("SerialNumber");
    result.loci[0].value = loci_data.serial_number;
    result.loci[1].key = strdup("WorldWideName");
    result.loci[1].value = loci_data.world_wide_name;
    result.loci[2].key = strdup("Model");
    result.loci[2].value = loci_data.model;
    result.loci[3].key = strdup("ControllerID");
    result.loci[3].value = loci_data.controller_id;
    result.loci[4].key = strdup("DiskShift");
    result.loci[4].value = loci_data.disk_shift;
    result.loci[5].key = strdup("eMMC_CID");
    result.loci[5].value = loci_data.emmc_cid;
    result.loci[6].key = strdup("UFS_UID");
    result.loci[6].value = loci_data.ufs_uid;
    result.loci[7].key = strdup("RAID_Controller_Model");
    result.loci[7].value = loci_data.raid_controller_model;
    result.loci[8].key = strdup("RAID_Volume_UID");
    result.loci[8].value = loci_data.raid_volume_uid;
    result.loci[9].key = strdup("TotalSize");
    result.loci[9].value = loci_data.total_size;
    result.loci[10].key = strdup("RotationRate");
    result.loci[10].value = loci_data.rotation_rate;
    result.loci[11].key = strdup("SectorSizes");
    result.loci[11].value = loci_data.sector_sizes;
    result.loci[12].key = strdup("FirmwareRevision");
    result.loci[12].value = loci_data.firmware_revision;
    result.loci[13].key = strdup("StandardsVersion");
    result.loci[13].value = loci_data.standards_version;
    result.loci[14].key = strdup("SupportedFeaturesHash");
    result.loci[14].value = loci_data.supported_features_hash;
    result.loci[15].key = strdup("RAID_Level");
    result.loci[15].value = loci_data.raid_level;
    result.loci[16].key = strdup("DiskGUID");
    result.loci[16].value = loci_data.disk_guid;
    result.loci[17].key = strdup("LastSectorHash");
    result.loci[17].value = loci_data.last_sector_hash;

    return result;
}

void LOCiD_FreeResult(LOCiD_Result* result) {
    if (result) {
        free(result->id_string);
    }
}

void LOCiD_FreeLociList(LOCiD_LociList* list) {
    if (list) {
        for (int i = 0; i < list->loci_count; i++) {
            free(list->loci[i].key);
            free(list->loci[i].value);
        }
        free(list->loci);
    }
}
