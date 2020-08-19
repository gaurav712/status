#include "network.h"
#include "battery.h"
#include "volume.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define SEPARATOR_SYMBOL    "  |  "
#define WIFI_ENABLED_SYMBOL "\uea0e"
#define WIFI_DISABLED_SYMBOL    "\uea0f"
#define RFKILL_DEV_NAME_LEN     7
#define BATTERY_NAME_LEN    5
#define BATTERY_STATUS_LEN  12
#define BATTERY_CHARGING_SYMBOL  "\ue91d"
#define BATTERY_DISCHARGING_SYMBOL  "\ue91c"
#define BATTERY_DISCHARGING_STATE   "Discharging"

int main(void) {

    char rfkill_device[RFKILL_DEV_NAME_LEN], battery_name[BATTERY_NAME_LEN], battery_status[BATTERY_STATUS_LEN];
    float down_bytes, up_bytes;
    short battery_capacity;
    time_t current_time = time(NULL);
    struct tm tm = *localtime(&current_time);

    const char *days_of_week[] = {
        "Sun",
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat"
    };

    const char *months_of_year[] = {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
    };

    /* -----VOLUME----- */
    printf("\uea0b %hd%%", get_volume());
    printf(SEPARATOR_SYMBOL);

    /* -----BATTERY----- */

    get_battery_name(battery_name);

    battery_capacity = get_battery_capacity(battery_name);

    get_battery_status(battery_name, battery_status);

    if (!(strcmp(battery_status, BATTERY_DISCHARGING_STATE))) {
        printf(BATTERY_DISCHARGING_SYMBOL);
    } else {
        printf(BATTERY_CHARGING_SYMBOL);
    }

    printf(" %hd%%", battery_capacity);

    printf(SEPARATOR_SYMBOL);

    /* -----NETWORK----- */

    find_rfkill_device(rfkill_device);

    if(network_is_enabled(rfkill_device)) {

        /* -----Network is enabled----- */
        if(network_is_connected()) {

            /* -----Network is connected, calculate network speed----- */

            get_bytes_transferred(&down_bytes, &up_bytes);
            printf("%.2fkb/s \ue90c\ue914 %.2fkb/s", down_bytes, up_bytes);

        } else {

            /* -----Network disconnected----- */
            printf(WIFI_ENABLED_SYMBOL);
        }

    } else {

        /* -----Network is disabled----- */
        printf(WIFI_DISABLED_SYMBOL);
    }
    printf(SEPARATOR_SYMBOL);

    /* -----DATE----- */
    printf("%s, %s %02d", days_of_week[tm.tm_wday], months_of_year[tm.tm_mon], tm.tm_mday);
    printf(SEPARATOR_SYMBOL);

    /* -----TIME----- */
    printf("%02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

    return 0;
}
