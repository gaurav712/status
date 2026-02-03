#include "battery.h"

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int8_t get_battery_name(char *battery_name) {

  DIR *dirp;
  struct dirent *dir;
  int8_t found = 0;

  battery_name[0] = '\0';

  if ((dirp = opendir(POWER_SUPPLY_DIR)) == NULL) {
    if (errno != ENOENT) {
      perror("opendir() failed!");
    }
    return 0;
  }

  while ((dir = readdir(dirp)) != NULL) {
    if (dir->d_name[0] == '.') {
      continue;
    }

    if (!(strncmp(dir->d_name, BAT_NAME_PATTERN, strlen(BAT_NAME_PATTERN)))) {
      strncpy(battery_name, dir->d_name, strlen(dir->d_name) + 1);
      found = 1;
      break;
    }
  }

  if (!found && errno) {
    perror("readdir() failed!");
  }

  if ((closedir(dirp)) == -1) {
    perror("closedir() failed!");
  }

  return found;
}

int8_t get_battery_capacity(char *battery_name) {

  FILE *fp;
  char battery_path[PATH_MAX] = POWER_SUPPLY_DIR;
  int8_t capacity;

  strncat(battery_path, battery_name, strlen(battery_name) + 1);
  strncat(battery_path, BAT_CAPACITY_FILE, strlen(BAT_CAPACITY_FILE) + 1);

  if ((fp = fopen(battery_path, "r")) == NULL) {
    perror("fopen() error!");
    exit(1);
  }

  fscanf(fp, "%hhd", &capacity);

  if ((fclose(fp)) == EOF) {
    perror("fclose() error!");
    exit(1);
  }

  return capacity;
}

void get_battery_status(char *battery_name, char *battery_status) {

  char battery_path[PATH_MAX] = POWER_SUPPLY_DIR;
  FILE *fp;

  strncat(battery_path, battery_name, strlen(battery_name) + 1);
  strncat(battery_path, BAT_STATUS_FILE, strlen(BAT_STATUS_FILE) + 1);

  if ((fp = fopen(battery_path, "r")) == NULL) {
    perror("fopen() error!");
    exit(1);
  }

  fscanf(fp, "%s", battery_status);

  if ((fclose(fp)) == EOF) {
    perror("fclose() error!");
    exit(1);
  }
}
