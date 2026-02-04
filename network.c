#include "network.h"

#include <dirent.h>
#include <errno.h>
#include <ifaddrs.h>
#include <limits.h>
#include <linux/if.h>
#include <linux/limits.h>
#include <linux/wireless.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

char interface_name[IFALIASZ];

void find_rfkill_device(char *rfkill_device) {

  DIR *dirp;
  struct dirent *dir = NULL;
  char rfkill_device_dir_path[PATH_MAX];

  if ((dirp = opendir(RFKILL_DIR)) == NULL) {
    perror("opendir() failed!");
    exit(1);
  }

  while (1) { // loop through files in RFKILL_DIR

    strncpy(rfkill_device_dir_path, RFKILL_DIR, strlen(RFKILL_DIR) + 1);

    if ((dir = readdir(dirp)) == NULL) {
      if (errno) {
        perror("readdir() failed!");
        exit(1);
      } else {
        break;
      }
    } else {
      if (dir->d_name[0] == '.') { // skip . and ..
        continue;
      } else {
        strncat(rfkill_device_dir_path, "/", 2);
        strncat(rfkill_device_dir_path, dir->d_name, strlen(dir->d_name) + 1);

        if (is_device_wlan(rfkill_device_dir_path)) {
          strncpy(rfkill_device, dir->d_name, strlen(dir->d_name) + 1);
          break;
        } else {
          continue;
        }
      }
    }
  }

  if ((closedir(dirp)) == -1) {
    perror("closedir() failed!");
    exit(1);
  }
}

int8_t is_device_wlan(char *rfkill_device_dir_path) {

  FILE *fp;
  char rfkill_dev_type[PATH_MAX];

  strncat(rfkill_device_dir_path, RFKILL_DEV_TYPE_FILE,
          strlen(RFKILL_DEV_TYPE_FILE) + 1);

  if ((fp = fopen(rfkill_device_dir_path, "r")) == NULL) {
    perror("fopen() error!");
    exit(1);
  }

  fscanf(fp, "%s", rfkill_dev_type);

  if ((fclose(fp)) == EOF) {
    perror("fclose() error!");
    exit(1);
  }

  if (!(strncmp(rfkill_dev_type, RFKILL_DEV_WLAN,
                strlen(RFKILL_DEV_WLAN) + 1))) { // found wlan device
    return 1;
  }

  return 0;
}

int8_t network_is_enabled(char *rfkill_device) {

  FILE *fp;
  int8_t state = 0;
  char rfkill_dev_path[PATH_MAX];

  strncpy(rfkill_dev_path, RFKILL_DIR, strlen(RFKILL_DIR) + 1);
  strncat(rfkill_dev_path, "/", 2);
  strncat(rfkill_dev_path, rfkill_device, strlen(rfkill_device) + 1);
  strncat(rfkill_dev_path, RFKILL_DEV_STATE_FILE,
          strlen(RFKILL_DEV_STATE_FILE) + 1);

  if ((fp = fopen(rfkill_dev_path, "r")) == NULL) {
    perror("fopen() error!");
    exit(1);
  }

  fscanf(fp, "%hhd", &state);

  if ((fclose(fp)) == EOF) {
    perror("fclose() error!");
    exit(1);
  }

  return (state);
}

int8_t network_is_connected(void) {

  FILE *fp;
  char dev_state[NET_DEVICE_STATE_LEN];
  char rfkill_dev_state_path[PATH_MAX];

  get_wireless_network_interface_name();

  strncpy(rfkill_dev_state_path, NET_DEVICES_DIR, strlen(NET_DEVICES_DIR) + 1);
  strncat(rfkill_dev_state_path, interface_name, strlen(interface_name) + 1);
  strncat(rfkill_dev_state_path, NET_DEVICE_STATE_FILE,
          strlen(NET_DEVICE_STATE_FILE) + 1);

  if ((fp = fopen(rfkill_dev_state_path, "r")) == NULL) {
    perror("fopen() error!");
    exit(1);
  }

  fscanf(fp, "%s", dev_state);

  if ((fclose(fp)) == EOF) {
    perror("fclose() error!");
    exit(1);
  }

  if (!strncmp(dev_state, NET_DEVICE_STATE_UP, NET_DEVICE_STATE_LEN))
    return 1;

  return 0;
}

void get_wireless_network_interface_name(void) {

  struct ifaddrs *ifa;

  if (getifaddrs(&ifa) == -1) {
    perror("getifaddrs() failed!");
    exit(1);
  }

  while (ifa->ifa_next != NULL) {
    if (interface_is_wireless(ifa->ifa_name)) {
      strncpy(interface_name, ifa->ifa_name, IFNAMSIZ);
      return;
    }

    ifa = ifa->ifa_next;
  }

  fprintf(stderr, "no wireless interfaces!\n");
  exit(1);
}

/*
 * This is a pretty neat hack by a guy named "Edu Felipe" to check if an
 * interface is wireless. Check it out at
 * https://gist.github.com/edufelipe/6108057
 */

int8_t interface_is_wireless(const char *device) {

  int sock = -1;
  struct iwreq iw;
  char protocol[IFNAMSIZ];

  memset(&iw, 0, sizeof(iw));
  strncpy(iw.ifr_name, device, IFNAMSIZ);

  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("socket() failed!");
    exit(1);
  }

  if (ioctl(sock, SIOCGIWNAME, &iw) != -1) {
    strcpy(protocol, iw.u.name);
    close(sock);
    return 1;
  }

  close(sock);
  return 0;
}

void get_bytes_transferred(float *down_bytes, float *up_bytes) {

  FILE *up_bytes_file, *down_bytes_file;
  uint64_t bytes_sent, bytes_sent_after_interval, bytes_received,
      bytes_received_after_interval;
  char up_path[PATH_MAX], down_path[PATH_MAX];

  strncpy(up_path, NET_DEVICES_DIR, strlen(NET_DEVICES_DIR) + 1);
  strncat(up_path, interface_name, strlen(interface_name) + 1);

  strncpy(down_path, up_path, strlen(up_path) + 1);

  strncat(down_path, NET_DEVICE_DOWN_BYTES_FILE,
          strlen(NET_DEVICE_DOWN_BYTES_FILE) + 1);
  strncat(up_path, NET_DEVICE_UP_BYTES_FILE,
          strlen(NET_DEVICE_UP_BYTES_FILE) + 1);

  if ((down_bytes_file = fopen(down_path, "r")) == NULL) {
    perror("fopen() failed!");
    exit(1);
  }

  if ((up_bytes_file = fopen(up_path, "r")) == NULL) {
    perror("fopen() failed!");
    exit(1);
  }

  fscanf(down_bytes_file, "%ld", &bytes_received);
  fscanf(up_bytes_file, "%ld", &bytes_sent);

  usleep(WAIT_TIME_MICROSECONDS);

  rewind(down_bytes_file);
  rewind(up_bytes_file);

  fscanf(down_bytes_file, "%ld", &bytes_received_after_interval);
  fscanf(up_bytes_file, "%ld", &bytes_sent_after_interval);

  *down_bytes = (bytes_received_after_interval - bytes_received) / 512.0;
  *up_bytes = (bytes_sent_after_interval - bytes_sent) / 512.0;

  if ((fclose(down_bytes_file)) == EOF) {
    perror("fclose() error!");
    exit(1);
  }

  if ((fclose(up_bytes_file)) == EOF) {
    perror("fclose() error!");
    exit(1);
  }
}
