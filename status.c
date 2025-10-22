#include "battery.h"
#include "network.h"
#include "volume.h"
#include "bluetooth.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define SEPARATOR_SYMBOL " : "
#define VOLUME_SYMBOL "\uf485"
#define MUTE_SYMBOL "\uf466"
#define DOWNLOAD_SYMBOL "\uf019"
#define UPLOAD_SYMBOL "\uf093 "
#define WIFI_ENABLED_SYMBOL "\uf1eb "
#define WIFI_DISABLED_SYMBOL "\uf072 "
#define RFKILL_DEV_NAME_LEN 7
#define BATTERY_NAME_LEN 5
#define BATTERY_STATUS_LEN 12
#define BATTERY_CHARGING_SYMBOL "\uf0e7"
#define BATTERY_EMPTY_SYMBOL "\U000f007a"
#define BATTERY_QUARTER_SYMBOL "\U000f007c"
#define BATTERY_HALF_SYMBOL "\U000f007e"
#define BATTERY_THREE_QUARTERS_SYMBOL "\U000f0080"
#define BATTERY_FULL_SYMBOL "\U000f0079"
#define BLUETOOTH_SYMBOL "\uf294"
#define BLUETOOTH_CONNECTED_SYMBOL "\uf294"

#define BATTERY_DISCHARGING_STATE "Discharging"

int main(void) {

    char rfkill_device[RFKILL_DEV_NAME_LEN], battery_name[BATTERY_NAME_LEN],
        battery_status[BATTERY_STATUS_LEN], bluetooth_rfkill_device[BLUETOOTH_RFKILL_DEV_NAME_LEN],
        bluetooth_device_name[BLUETOOTH_DEVICE_NAME_LEN];
    float down_bytes, up_bytes;
    int mute;
    short battery_capacity, volume;
    time_t current_time = time(NULL);
    struct tm tm = *localtime(&current_time);

    const char *days_of_week[] = {"Sun", "Mon", "Tue", "Wed",
                                  "Thu", "Fri", "Sat"};

    const char *months_of_year[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    /* -----VOLUME----- */
    if (get_mute()) {
        printf("%s %hd%%", VOLUME_SYMBOL, get_volume());
    } else {
        printf(MUTE_SYMBOL);
    }
    printf(SEPARATOR_SYMBOL);

    /* -----BATTERY----- */

    get_battery_name(battery_name);

    get_battery_status(battery_name, battery_status);
    battery_capacity = get_battery_capacity(battery_name);

    if (strcmp(battery_status, BATTERY_DISCHARGING_STATE)) {
        printf("%s", BATTERY_CHARGING_SYMBOL);
    } else {
        if (battery_capacity < 20) {
            printf(BATTERY_EMPTY_SYMBOL);
        } else if (battery_capacity < 40) {
            printf(BATTERY_QUARTER_SYMBOL);
        } else if (battery_capacity < 60) {
            printf(BATTERY_HALF_SYMBOL);
        } else if (battery_capacity < 80) {
            printf(BATTERY_THREE_QUARTERS_SYMBOL);
        } else {
            printf(BATTERY_FULL_SYMBOL);
        }
    }

    printf(" %hd%%", battery_capacity);

    printf(SEPARATOR_SYMBOL);

    /* -----NETWORK----- */

    find_rfkill_device(rfkill_device);

    if (network_is_enabled(rfkill_device)) {

        /* -----Network is enabled----- */
        if (network_is_connected()) {

            /* -----Network is connected, calculate network speed----- */

            get_bytes_transferred(&down_bytes, &up_bytes);
            printf("%.2fkb/s %s %s %.2fkb/s", down_bytes, DOWNLOAD_SYMBOL,
                   UPLOAD_SYMBOL, up_bytes);

        } else {

            /* -----Network disconnected----- */
            printf(WIFI_ENABLED_SYMBOL);
        }

    } else {

        /* -----Network is disabled----- */
        printf(WIFI_DISABLED_SYMBOL);
    }
    printf(SEPARATOR_SYMBOL);

    /* -----BLUETOOTH----- */

    find_bluetooth_rfkill_device(bluetooth_rfkill_device);

    if (bluetooth_is_enabled(bluetooth_rfkill_device)) {

        /* -----Bluetooth is enabled----- */
        if (bluetooth_is_connected()) {

            /* -----Bluetooth is connected, get device name----- */
            get_connected_bluetooth_device_name(bluetooth_device_name);
            printf("%s %s", BLUETOOTH_CONNECTED_SYMBOL, bluetooth_device_name);

        } else {

            /* -----Bluetooth enabled but not connected----- */
            printf("%s", BLUETOOTH_SYMBOL);
        }

    } else {

        /* -----Bluetooth is disabled----- */
        printf("%s", BLUETOOTH_SYMBOL);
    }
    printf(SEPARATOR_SYMBOL);

    /* -----DATE----- */
    printf("%s, %s %02d", days_of_week[tm.tm_wday], months_of_year[tm.tm_mon],
           tm.tm_mday);
    printf(SEPARATOR_SYMBOL);

    /* -----TIME----- */
    printf("%02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

    return 0;
}
