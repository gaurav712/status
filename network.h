#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>

#define RFKILL_DIR "/sys/class/rfkill/"
#define RFKILL_DEV_TYPE_FILE "/type"
#define RFKILL_DEV_STATE_FILE "/state"
#define RFKILL_DEV_WLAN "wlan"

#define NET_DEVICES_DIR "/sys/class/net/"
#define NET_DEVICE_STATE_FILE "/operstate"
#define NET_DEVICE_STATE_UP "up"
#define NET_DEVICE_STATE_LEN 5
#define NET_DEVICE_UP_BYTES_FILE "/statistics/tx_bytes"
#define NET_DEVICE_DOWN_BYTES_FILE "/statistics/rx_bytes"

#define WAIT_TIME_MICROSECONDS 500000 // 0.5 seconds

extern int usleep (__useconds_t __useconds); // to shut up the compiler

void find_rfkill_device(char *rfkill_device);
int8_t is_device_wlan(char *rfkill_device_dir_path);
int8_t network_is_enabled(char *rfkill_device);
int8_t network_is_connected(void);
void get_wireless_network_interface_name(void);
int8_t interface_is_wireless(const char *device);
void get_bytes_transferred(float *down_bytes, float *up_bytes);

#endif // NETWORK_H
