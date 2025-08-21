#include "device.h"
#include <stddef.h>

#ifdef _WIN32

char* get_device_path(const char* mode, int number) {
    return NULL; // Not implemented for Windows
}

#else

#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Comparison function for qsort
static int compare_strings(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

char* get_device_path(const char* mode, int number) {
    if (strcmp(mode, "pd") != 0) {
        return NULL; // Only pd mode is supported for now
    }

    DIR* d = opendir("/sys/block");
    if (!d) {
        return NULL;
    }

    struct dirent* dir;
    char* device_names[256];
    int count = 0;

    while ((dir = readdir(d)) != NULL) {
        // Skip . and ..
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
            continue;
        }
        // Skip loop devices
        if (strncmp(dir->d_name, "loop", 4) == 0) {
            continue;
        }
        device_names[count++] = strdup(dir->d_name);
    }
    closedir(d);

    if (count == 0) {
        return NULL;
    }

    qsort(device_names, count, sizeof(char*), compare_strings);

    if (number >= 0 && number < count) {
        char* path = malloc(256);
        sprintf(path, "/dev/%s", device_names[number]);
        for (int i = 0; i < count; i++) {
            free(device_names[i]);
        }
        return path;
    } else {
        for (int i = 0; i < count; i++) {
            free(device_names[i]);
        }
        return NULL;
    }
}

#endif
