#include "battery.h"
#include "bluetooth.h"
#include "network.h"
#include "volume.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define SEPARATOR_SYMBOL " : "

const char *VolumeIcons[] = {
    "\uf485",     // SPEAKER
    "\U000F02CB", // HEADPHONE
    "\U000F00B0", // BT_HEADSET
    "\uf466"      // MUTE
};

// VolumeIcon enum defined in volume.h

#define RFKILL_DEV_NAME_LEN 10

const char *NetworkIcons[] = {
    "\uf1eb", // ENABLED
    "\uf072", // DISABLED
    "\uf019", // DOWNLOAD
    "\uf093 " // UPLOAD
};

enum NetworkIcon { IC_NT_ENABLED, IC_NT_DISABLED, IC_DOWNLOAD, IC_UPLOAD };

#define BAT_NAME_LEN 5
#define BAT_STATUS_LEN 12
#define BAT_CHARGING_STATE "Charging"

const char *BatteryIcons[] = {
    "\U000f007a", // EMPTY
    "\U000f007c", // QUARTER
    "\U000f007e", // HALF
    "\U000f0080", // THREE_QUARTERS
    "\U000f0079", // FULL
    "\uf0e7"      // CHARGING
};

enum BatteryIcon {
  IC_BAT_EMPTY,
  IC_BAT_25,
  IC_BAT_50,
  IC_BAT_75,
  IC_BAT_100,
  IC_BAT_CHARGING
};

#define BLUETOOTH_DEVICE_NAME_LEN 50

const char *BluetoothIcons[] = {
    "\uf294",     // ENABLED
    "\U000F00B1", // CONNECTED
    "\U000F00B2"  // DISABLED
};

enum BluetoothIcon { IC_BT_ENABLED, IC_BT_CONNECTED, IC_BT_DISABLED };

#define DAYS_OF_WEEK {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"}
#define MONTHS_OF_YEAR                                                         \
  {"Jan", "Feb", "Mar", "Apr", "May", "Jun",                                   \
   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}

int main(void) {

  char rfkill_device[RFKILL_DEV_NAME_LEN];
  char battery_name[BAT_NAME_LEN];
  char battery_status[BAT_STATUS_LEN];
  char bluetooth_device_name[BLUETOOTH_DEVICE_NAME_LEN];

  float down_bytes, up_bytes;
  int8_t battery_capacity;

  time_t current_time = time(NULL);
  struct tm tm = *localtime(&current_time);

  const char *days_of_week[] = DAYS_OF_WEEK;

  const char *months_of_year[] = MONTHS_OF_YEAR;

  /* -----VOLUME----- */

  if (!get_mute()) {
    enum VolumeIcon icon_type = get_volume_icon_type();
    printf("%s %hd%%", VolumeIcons[icon_type], get_volume());
  } else {
    printf("%s", VolumeIcons[IC_MUTE]);
  }

  printf(SEPARATOR_SYMBOL);

  /* -----BATTERY----- */

  if (get_battery_name(battery_name)) {
    get_battery_status(battery_name, battery_status);
    battery_capacity = get_battery_capacity(battery_name);

    if (strncmp(battery_status, BAT_CHARGING_STATE, BAT_STATUS_LEN) == 0) {
      printf("%s", BatteryIcons[IC_BAT_CHARGING]);
    } else {
      if (battery_capacity < 20) {
        printf("%s", BatteryIcons[IC_BAT_EMPTY]);
      } else if (battery_capacity < 40) {
        printf("%s", BatteryIcons[IC_BAT_25]);
      } else if (battery_capacity < 60) {
        printf("%s", BatteryIcons[IC_BAT_50]);
      } else if (battery_capacity < 80) {
        printf("%s", BatteryIcons[IC_BAT_75]);
      } else {
        printf("%s", BatteryIcons[IC_BAT_100]);
      }
    }

    printf(" %hd%%", battery_capacity);
  }

  printf(SEPARATOR_SYMBOL);

  /* -----NETWORK----- */

  find_rfkill_device(rfkill_device);

  if (network_is_enabled(rfkill_device)) {
    if (network_is_connected()) {
      get_bytes_transferred(&down_bytes, &up_bytes);
      printf("%.2fkb/s %s %s %.2fkb/s", down_bytes, NetworkIcons[IC_DOWNLOAD],
             NetworkIcons[IC_UPLOAD], up_bytes);
    } else {
      printf("%s", NetworkIcons[IC_NT_ENABLED]); // Diconnected
    }
  } else {
    printf("%s", NetworkIcons[IC_NT_DISABLED]); // Network disabled
  }

  printf(SEPARATOR_SYMBOL);

  /* -----BLUETOOTH----- */

  if (bluetooth_is_blocked()) {
    printf("%s", BluetoothIcons[IC_BT_DISABLED]); // Bluetooth disabled

  } else {

    if (bluetooth_is_connected()) {
      get_connected_bluetooth_device_name(bluetooth_device_name);

      char *battery_info = get_connected_bluetooth_device_battery();

      if (bluetooth_device_name[0] != '\0') {
        if (battery_info[0] != '\0') {
          printf("%s %s (%s)", BluetoothIcons[IC_BT_CONNECTED],
                 bluetooth_device_name, battery_info);
        } else {
          printf("%s %s", BluetoothIcons[IC_BT_CONNECTED],
                 bluetooth_device_name);
        }
      } else {
        printf("%s", BluetoothIcons[IC_BT_CONNECTED]);
      }
    } else {
      printf("%s", BluetoothIcons[IC_BT_ENABLED]); // Enabled, not connected
    }
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
