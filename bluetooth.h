#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <unistd.h>
#include <sys/stat.h>

#define BLUETOOTH_DEVICE_NAME_LEN 50
#define BLUETOOTH_RFKILL_DEV_NAME_LEN 10

/* Function declarations */
void find_bluetooth_rfkill_device(char *rfkill_device);
short bluetooth_is_enabled(char *rfkill_device);
short bluetooth_is_blocked(void);
short bluetooth_is_connected(void);
void get_connected_bluetooth_device_name(char *device_name);

#endif
