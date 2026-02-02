#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>

int8_t get_battery_name(char *battery_name);
int8_t get_battery_capacity(char *battery_name);
void get_battery_status(char *battery_name, char *battery_status);

#endif // BATTERY_H
