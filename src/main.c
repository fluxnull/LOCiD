#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "hashing.h"
#include "loci.h"
#include "stability.h"
#include "locid.h"
#include "globals.h"
#include "device.h"

void print_help() {
    printf("Usage: locid [options]\n");
    printf("Options:\n");
    printf("  -h, --help\t\tShow this help message\n");
    printf("  --volatility N\tSet volatility level (0 or 1)\n");
    printf("  --loci L\t\tProvide a comma-separated list of Loci numbers\n");
}

int main(int argc, char *argv[]) {
    int volatility = 0; // Default volatility
    char* loci_selection = NULL;
    int chars = 256; // Default hash length
    bool json_output = false;
    bool stability_output = false;
    char* device_mode = "pd";
    int device_number = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help();
            return 0;
        } else if (strcmp(argv[i], "--volatility") == 0) {
            if (i + 1 < argc) {
                volatility = atoi(argv[++i]);
                if (volatility != 0 && volatility != 1) {
                    fprintf(stderr, "Error: Invalid value for --volatility. Must be 0 or 1.\n");
                    return 1;
                }
            }
        } else if (strcmp(argv[i], "--loci") == 0) {
            if (i + 1 < argc) {
                loci_selection = argv[++i];
            }
        } else if (strcmp(argv[i], "--chars") == 0) {
            if (i + 1 < argc) {
                chars = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "--json") == 0) {
            json_output = true;
        } else if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-V") == 0) {
            verbose_output = true;
        } else if (strcmp(argv[i], "--stability") == 0) {
            stability_output = true;
        } else if (strcmp(argv[i], "--device") == 0) {
            if (i + 2 < argc) {
                device_mode = argv[++i];
                device_number = atoi(argv[++i]);
            }
        }
    }

    char* device_path = get_device_path(device_mode, device_number);
    if (!device_path) {
        fprintf(stderr, "Error: Device not found.\n");
        return 2;
    }

    bool list_option = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--list") == 0) {
            list_option = true;
            break;
        }
    }

    if (list_option) {
        LOCiD_LociList list = LOCiD_List(device_path, volatility, loci_selection);
        if (list.error_code) {
            fprintf(stderr, "Error: Could not list loci. Error code: %d\n", list.error_code);
            return list.error_code;
        }
        if (json_output) {
            printf("{\n");
            for (int i = 0; i < list.loci_count; i++) {
                printf("  \"%s\": \"%s\"", list.loci[i].key, list.loci[i].value);
                if (i < list.loci_count - 1) {
                    printf(",\n");
                } else {
                    printf("\n");
                }
            }
            printf("}\n");
        } else {
            printf("%-25s | %s\n", "Locus", "Value");
            printf("--------------------------|--------------------------------------------------\n");
            for (int i = 0; i < list.loci_count; i++) {
                printf("%-25s | %s\n", list.loci[i].key, list.loci[i].value);
            }
        }
        int na_count = 0;
        for (int i = 0; i < list.loci_count; i++) {
            if (strcmp(list.loci[i].value, "_NA_") == 0) {
                na_count++;
            }
        }

        LOCiD_FreeLociList(&list);

        if (na_count == list.loci_count) {
            return 4; // Critical read failure
        } else if (na_count > 0) {
            return 5; // Partial success
        } else {
            return 0; // Success
        }
    } else {
        printf("LOCiD Tool v0.1.0\n");
        char* rank = NULL;
        int na_count = 0;
        LOCiD_Result result = LOCiD_Generate(device_path, chars, volatility, loci_selection, stability_output ? &rank : NULL, &na_count);

        if (result.error_code) {
            fprintf(stderr, "Error: Could not generate LOCiD. Error code: %d\n", result.error_code);
            return result.error_code;
        }

        if (stability_output) {
            if (json_output) {
                printf("{\"locid\": \"%s\", \"stability\": \"%s\"}\n", result.id_string, rank);
            } else {
                printf("LOCiD: %s [Stability: %s]\n", result.id_string, rank);
            }
            free(rank);
        } else {
            if (json_output) {
                printf("{\"locid\": \"%s\"}\n", result.id_string);
            } else {
                printf("LOCiD: %s\n", result.id_string);
            }
        }
        LOCiD_FreeResult(&result);

        if (na_count == 18) {
            return 4; // Critical read failure
        } else if (na_count > 0) {
            return 5; // Partial success
        }
    }

    free(device_path);
    return 0;
}
