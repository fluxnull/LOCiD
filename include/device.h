#ifndef DEVICE_H
#define DEVICE_H

// Gets the device path for a given mode and number.
// The caller is responsible for freeing the returned string.
char* get_device_path(const char* mode, int number);

#endif // DEVICE_H
