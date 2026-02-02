#ifndef BLUETOOTH_H
#define BLUETOOTH_H

void find_bluetooth_rfkill_device(char *rfkill_device);
short bluetooth_is_enabled(char *rfkill_device);
short bluetooth_is_blocked(void);
short bluetooth_is_connected(void);
void get_connected_bluetooth_device_name(char *device_name);
char* get_connected_bluetooth_device_battery(void);

#endif // BLUETOOTH_H
