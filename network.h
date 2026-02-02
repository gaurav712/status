#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>

void find_rfkill_device(char *rfkill_device);
void check_device_type(char *rfkill_device_dir_path);
int8_t network_is_enabled(char *rfkill_device);
int8_t network_is_connected(void);
void get_wireless_network_interface_name(void);
int8_t interface_is_wireless(const char *device);
void get_bytes_transferred(float *down_bytes, float *up_bytes);

#endif // NETWORK_H
