#ifndef BATTERY_H
#define BATTERY_H
int get_battery_name(char *battery_name);

short get_battery_capacity(char *battery_name);

void get_battery_status(char *battery_name, char *battery_status);
#endif // BATTERY_H
