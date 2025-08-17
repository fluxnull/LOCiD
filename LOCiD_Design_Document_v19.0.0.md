# LOCiD Tool — Design and Implementation Documentation (v19.0.0)

## 1. Introduction

The **LOCiD** tool is a professional-grade executable and library that generates a **unique, reproducible identifier for storage devices**. It is designed to function as both a **standalone command-line utility (`LOCiD.exe`)** and a **linked library (LOCiD.dll/.so)** for integration into other applications.

LOCiD anchors a device's identity to its immutable and logical **loci**—the specific, measurable anchor points of its hardware and software configuration—to produce a single, resilient fingerprint.

This definitive specification provides a stable feature set, offering both simple, powerful presets (`--volatility`) and granular, expert-level control (`--loci`) to solve the most challenging real-world identification problems.

---

## 2. Naming Origin: LOCI + ID

The name **LOCiD** is a fusion of two concepts: **LOCI** and **ID**, with the "i" shared to represent their synthesis.

-   **LOCI**: The plural of *locus*, meaning places, positions, or anchors.
-   **ID**: Identity.

The name literally encodes the tool's core concept: **"The identity that comes from loci."**

---

## 3. Core Hashing Specification

The integrity of the LOCiD fingerprint is defined by a precise and transparent hashing methodology.

### 3.1 Hash Algorithm: BLAKE3
-   **Algorithm**: LOCiD uses the **BLAKE3** cryptographic hash function.
-   **Internal Hash Size**: A fixed internal hash size of **1024 bits** is used (achieved via BLAKE3's extended output mode, e.g., using blake3_hasher_finalize_seek in the reference implementation to generate longer digests).
-   **Rationale**: BLAKE3 is chosen for its exceptional speed, massive parallelism, and strong, vetted cryptographic security. It is chosen over non-cryptographic hashes like xxHash because LOCiD is intended for security-sensitive applications where collision resistance is mandatory.

---

## 4. Loci Filtering & Control

LOCiD offers two primary modes of operation for selecting which loci contribute to the fingerprint.

### 4.1 Preset Volatility Levels (`--volatility`)
This section describes the two primary, preset modes for generating a LOCiD, designed for the most common use cases.

#### `volatility = 0` (Hardware + Logic)
-   **Description**: This is the **default and most comprehensive** mode. It includes **all applicable loci** (dynamically filtered by device type, e.g., skipping eMMC_CID for SSDs or RAID fields for non-RAID drives), combining stable hardware identifiers (Immutable and Stateful categories) with logical ones (`DiskGUID`, `LastSectorHash` if applicable).
-   **Philosophy**: This mode provides the most unique and resilient fingerprint. It tells the most complete story of the drive by capturing both its physical identity and its current logical state. It is particularly robust in "shitty enclosure" scenarios where many hardware loci may be unavailable, as it can still generate a strong, stable ID from the surviving logical data. Device type is detected (e.g., via Model, RotationRate, or query methods) to exclude irrelevant fields, reducing unnecessary _NA_.
-   **Trade-off**: The resulting ID **will change** if the drive is fully wiped and re-initialized, as this action explicitly alters the logical loci (#17 and #18).

#### `volatility = 1` (Hardware Only)
-   **Description**: This mode includes **only applicable hardware-based loci** (dynamically filtered by device type from Immutable and Stateful categories). It explicitly **excludes** the two Logical loci: `#17 (DiskGUID)` and `#18 (LastSectorHash)`, and skips type-irrelevant fields (e.g., no DiskShift for SSDs).
-   **Philosophy**: This mode is designed to generate an identifier that is tied exclusively to the physical hardware and its configured state. It is intentionally "blind" to any OS-level formatting or data layout. Device type detection ensures only relevant fields are attempted, minimizing degradation.
-   **Use Case**: This mode is ideal for pure hardware asset tracking. The generated ID is designed to remain constant even if the drive is completely erased, re-partitioned, or has a new operating system installed, as none of those actions alter the 16 hardware loci.

### 4.2 Granular Selection (`--loci`)
This is the expert mode, designed for edge cases where the presets are insufficient.
-   **Functionality**: The `--loci` flag accepts a comma-separated list of locus numbers or ranges from the Attribute Loci Matrix (e.g., "1,3-5,10" for #1, #3 to #5, #10). This allows the user to hand-pick the exact set of attributes to build the fingerprint.
-   **Absolute Precedence**: If the `--loci` flag is used, it **always overrides** any `--volatility` setting. It provides the ultimate level of control for situations like ignoring a known-bad serial number.

---

## 5. Attribute Loci Matrix

This matrix is the definitive, ordered list of all attributes LOCiD will attempt to collect.

| # | Field / Locus Name | Category | Description / Role in Fingerprint | Availability & Query Method | Impact of Bad USB Enclosure | Estimated Probability of Change (%) | Drive Type Notes |
| :-- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **1** | **SerialNumber** | Immutable | The manufacturer's official serial number. | SATA/NVMe (IOCTL/WMI on Windows; ioctl on Linux/Mac), RAID Controllers, some eMMC | Likely Blocked/Generic | <0.001% | Full on RAID if exposed. |
| **2** | **World Wide Name (WWN)**| Immutable | A globally unique identifier burned into the firmware. | SATA/NVMe (IOCTL on Windows; ioctl on Linux/Mac), Enterprise SAS/FC | Likely Blocked | <0.001% | No eMMC equiv. |
| **3** | **Model** | Immutable | The manufacturer-assigned model number. | Universal (WMI on Windows, sysfs/udev on Linux, IOKit on Mac, RAID API) | Likely Blocked/Generic | <0.001% | Consistent on RAID. |
| **4** | **ControllerID (NVMe)** | Immutable | A unique static identifier for the NVMe controller. | NVMe Only (IOCTL on Windows; ioctl on Linux/Mac) | Will Fail | <0.001% | N/A RAID virtual. |
| **5** | **DiskShift (SMART 220)** | Immutable | A factory-calibrated physical measurement. | HDDs Primarily (SMART Passthrough via IOCTL on Windows, smartctl/ioctl on Linux/Mac) | Likely Blocked | ~0.001% | HDD-only. |
| **6** | **eMMC_CID** | Immutable | The 128-bit Card Identification register hash. | eMMC, SD Cards (Platform-specific IOCTL on Windows; mmc-utils/ioctl on Linux/Mac) | Varies, often succeeds | <0.001% | eMMC-specific. |
| **7** | **UFS_UID** | Immutable | A hash of unique vendor/model/serial identifiers. | UFS (Platform-specific IOCTL on Windows; ufs-utils/ioctl on Linux/Mac) | Varies | <0.001% | UFS-specific. |
| **8** | **RAID_Controller_Model** | Immutable | The model of the hardware RAID controller. | Hardware RAID Logical Volumes (Vendor API like storcli on Windows/Linux/Mac) | N/A | <0.001% | Hardware RAID. |
| **9** | **RAID_Volume_UID** | Immutable | A unique identifier assigned by the RAID controller. | Hardware RAID Logical Volumes (Vendor API like storcli on Windows/Linux/Mac) | N/A | <0.001% | Hardware RAID. |
| **10** | **TotalSize** | Immutable | The total raw capacity of the drive. | Universal (IOCTL_DISK_GET_LENGTH_INFO on Windows; ioctl on Linux/Mac) | Almost Always Succeeds | ~0.01% | Reports array size on RAID. |
| **11** | **RotationRate** | Immutable | Platter spin rate or solid-state flag. | HDDs & SSDs (SMART Passthrough via IOCTL on Windows, smartctl/ioctl on Linux/Mac) | Likely Blocked | ~0.01% | Partial RAID. |
| **12** | **SectorSizes** | Immutable | The logical and physical sector sizes. | Universal (IOCTL_STORAGE_QUERY_PROPERTY on Windows; ioctl on Linux/Mac) | Often Succeeds | ~0.01% | Consistent RAID/eMMC. |
| **13** | **FirmwareRevision** | Stateful | The firmware version. | Nearly Universal (WMI on Windows, sysfs/udev on Linux, IOKit on Mac, RAID API) | Likely Blocked/Generic | ~0.01% | Per-drive RAID. |
| **14** | **StandardsVersion** | Stateful | The ATA/NVMe specification version supported. | SATA & NVMe (IDENTIFY DEVICE command via IOCTL on Windows, ioctl on Linux/Mac) | Likely Blocked | ~0.01% | N/A eMMC direct. |
| **15** | **SupportedFeaturesHash** | Stateful | A hash of the controller's supported features (TRIM, etc.). | SATA & NVMe (IDENTIFY DEVICE command via IOCTL on Windows, ioctl on Linux/Mac) | Likely Blocked | ~0.01% | Expandable RAID. |
| **16** | **RAID_Level** | Stateful | The configured RAID level (e.g., RAID 5). | Hardware & Software RAID (Vendor API like storcli, mdadm on Linux/Mac) | N/A | ~0.01% | Hardware/software RAID. |
| **17** | **DiskGUID** | Logical | The unique identifier for the logical partition scheme. | Any drive with GPT (Raw sector read via IOCTL on Windows, read on Linux/Mac) | Succeeds | ~5% | Works RAID volumes. |
| **18** | **LastSectorHash** | Logical | A Blake3 hash of the last physical sector. | Universal (Raw sector read via IOCTL on Windows, read on Linux/Mac) | Succeeds | ~5% | Logs errors; reliable RAID. |

---

## 6. Implementation & Error Handling

- **Language**: Portable C, with bindings for C++/Go.
- **Hashing**: BLAKE3 reference implementation.
- **Platforms**: Windows (WMI/IOCTL), Linux (/sys/block, udev, ioctl), macOS (IOKit in roadmap).
- **Querying Loci**: Direct hardware queries; RAID via vendor APIs (e.g., storcli for MegaRAID, mdadm for software); eMMC/UFS via mmc-utils/ufs-utils equivalents.
- **Graceful Degradation**: If a locus fails or is inapplicable, use "_NA_" in input string.
- **Read Failures**: For LastSectorHash, fallback to Blake3 of TotalSize; log to stderr/verbose.
- **Collision Risks**: Full 1024-bit hash: ~1 in 2^512 collisions theoretically; degraded (many _NA_): ~1 in 10^6 for similar setups—use --list to inspect; recommend full loci for uniqueness.
- **Bad Enclosure Handling**: _NA_ for blocked fields; tested with cheap USB bridges showing consistent but distinct IDs.
- **Partial Metadata Handling**: If any selected loci return _NA_ (after dynamic filtering), output a message to stderr: "Only partial metadata was captured: we got a signature but it's missing field values." This triggers exit code 5 on success with degradation.

### 6.1 Exit Codes
- `0` → Success (full capture)
- `1` → Invalid arguments
- `2` → Device not found
- `3` → Permission denied (admin/root required)
- `4` → Critical read failure (no loci available)
- `5` → Partial success (ID generated but with missing fields; check verbose for details)

---

## 7. Command-Line Options

| Option | Alias(es) | Description |
| :--- | :--- | :--- |
| `-h, /?, --help` | | Show usage information and the full Loci Matrix. |
| `--volatility N` | `-v N` | **(Preset)** Set volatility level. `0` = Hardware + Logic (Default). `1` = Hardware Only. |
| `--loci L` | `-l L` | **(Expert Filter)** Provide a comma-separated list of Loci numbers or ranges (e.g., `"1,3-5,10"`) to use for the fingerprint. **Overrides `--volatility`**. |
| `--list` | | **(Diagnostic Mode)** Dumps a table of all loci and their values for the target device and exits. |
| `--chars N` | `-c N` | Output length of the hash (1–256). Default: 256. |
| `--device MODE N` | `-d MODE N` | Select device by mode and number, where MODE is 'pd' (PhysicalDrive on Windows, /dev/sd* on Linux/Unix/Mac, default) or 'dn' (DiskNumber on Windows, numeric block on Linux/Unix/Mac), and N is the number or letter (e.g., --device pd 0 for \\.\PhysicalDrive0 on Windows or /dev/sda on Linux, --device dn 1 for DiskNumber 1 on Windows or equivalent Linux block). If MODE omitted, defaults to 'pd'. On Linux/Unix/Mac, 'pd' maps to physical block devices like /dev/sdN or /dev/nvme0n1, 'dn' to enumerated disk numbers via udev or IOKit; differences: Linux uses /dev paths directly, Mac uses /dev/diskN, with dynamic detection for type-specific queries (e.g., ioctl for ATA, nvme-cli for NVMe). |
| `--json` | | Use JSON format for output. |
| `--verbose` | `-V` | Dump detailed info on fields used, _NA_, errors to stderr. |
| `--stability` | | Rank the uniqueness quality of available loci (e.g., High/Medium/Low based on non-_NA_ count and change probabilities); appends rank to output, also included in --verbose. |

---

## 8. Validation and Testing Strategy

- **Unit Tests**: Each platform-specific data collection function unit-tested (CUnit, 100% coverage).
- **Determinism Validation**: Test harness on known hardware verifies output for presets and custom filters.
- **Filter Logic Test**: Confirms --loci overrides --volatility; hash from specified loci only.
- **Bad Enclosure Simulation**: Run against non-compliant USB bridges for degradation and _NA_ correctness.
- **Benchmarks**: <5ms on typical drives; collision sims on degraded sets.
- **Real-World**: 100+ drives (HDD/SSD/RAID/eMMC); CI/CD cross-OS.
- **Dynamic Filtering Test**: Verify loci selection changes per device type (e.g., SSD skips eMMC/RAID fields; HDD skips NVMe-specific).
- **Partial Metadata Test**: Simulate _NA_ scenarios (e.g., bad enclosure) to confirm exit code 5, stderr message "Only partial metadata was captured: we got a signature but it's missing field values", and ID still generates.

---

## 9. Future Roadmap
- **macOS Support**: Implement data collection using IOKit.
- **Batch Mode**: LOCiDs for all drives.
- **SQLite Audit Log**: Track known IDs.
- **WebAssembly Port**: Browser-based tools.

---

## 10. Library API

```c
#ifndef LOCID_H
#define LOCID_H

// Result struct for generating a LOCiD
typedef struct {
    char* id_string;           // Heap-allocated LOCiD. Must be freed.
    int error_code;            // 0 on success, 5 on partial.
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
LOCiD_Result LOCiD_Generate(const char* device_path, int chars, int volatility, const char* loci);

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
```