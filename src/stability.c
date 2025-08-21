#include "stability.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// These probabilities are from the Loci Matrix in the design document.
// They represent the estimated probability of change for each locus.
static const double change_probabilities[] = {
    0.0, // Dummy value for 0 index
    0.00001, // 1. SerialNumber
    0.00001, // 2. World Wide Name (WWN)
    0.00001, // 3. Model
    0.00001, // 4. ControllerID (NVMe)
    0.0001,  // 5. DiskShift (SMART 220)
    0.00001, // 6. eMMC_CID
    0.00001, // 7. UFS_UID
    0.00001, // 8. RAID_Controller_Model
    0.00001, // 9. RAID_Volume_UID
    0.0001,  // 10. TotalSize
    0.0001,  // 11. RotationRate
    0.0001,  // 12. SectorSizes
    0.0001,  // 13. FirmwareRevision
    0.0001,  // 14. StandardsVersion
    0.0001,  // 15. SupportedFeaturesHash
    0.0001,  // 16. RAID_Level
    0.05,    // 17. DiskGUID
    0.05,    // 18. LastSectorHash
};

// Computes the stability rank based on the formula from the checklist:
// score = sum(1 - change_prob for non-_NA_) / total;
// avail = non_na / total;
// final = score * avail;
// High >0.8, Medium >0.5, Low else.
char* compute_stability_rank(const Loci* loci) {
    const char* loci_values[] = {
        loci->serial_number,
        loci->world_wide_name,
        loci->model,
        loci->controller_id,
        loci->disk_shift,
        loci->emmc_cid,
        loci->ufs_uid,
        loci->raid_controller_model,
        loci->raid_volume_uid,
        loci->total_size,
        loci->rotation_rate,
        loci->sector_sizes,
        loci->firmware_revision,
        loci->standards_version,
        loci->supported_features_hash,
        loci->raid_level,
        loci->disk_guid,
        loci->last_sector_hash,
    };
    int num_loci = sizeof(loci_values) / sizeof(loci_values[0]);
    int non_na_count = 0;
    double score_sum = 0.0;

    for (int i = 0; i < num_loci; i++) {
        if (strcmp(loci_values[i], "_NA_") != 0) {
            non_na_count++;
            score_sum += (1.0 - change_probabilities[i + 1]);
        }
    }

    if (non_na_count == 0) {
        return strdup("Low");
    }

    double score = score_sum / non_na_count;
    double avail = (double)non_na_count / num_loci;
    double final_score = score * avail;

    if (final_score > 0.8) {
        return strdup("High");
    } else if (final_score > 0.5) {
        return strdup("Medium");
    } else {
        return strdup("Low");
    }
}
