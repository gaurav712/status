void find_rfkill_device(char *rfkill_device);

void check_device_type(char *rfkill_device_dir_path);

short network_is_enabled(char *rfkill_device);

short network_is_connected(void);

void get_wireless_network_interface_name(void);

short interface_is_wireless(const char *device);

void get_bytes_transferred(float *down_bytes, float *up_bytes);
