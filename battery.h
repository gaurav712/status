#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>

#define POWER_SUPPLY_DIR "/sys/class/power_supply/"
#define BAT_NAME_PATTERN "BAT"
#define BAT_CAPACITY_FILE "/capacity"
#define BAT_STATUS_FILE "/status"

int8_t get_battery_name(char *battery_name);
int8_t get_battery_capacity(char *battery_name);
void get_battery_status(char *battery_name, char *battery_status);

#endif // BATTERY_H
