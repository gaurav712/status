#include "bluetooth.h"

#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stddef.h>

#define RFKILL_DIR  "/sys/class/rfkill/"
#define RFKILL_DEV_TYPE_FILE_NAME   "/type"
#define RFKILL_DEV_TYPE_NAME   "bluetooth"
#define RFKILL_DEV_STATE_FILE_NAME  "/state"
#define BLUETOOTH_DEVICE_NAME_LEN 50
#define BLUETOOTH_RFKILL_DEV_NAME_LEN 10

char bluetooth_temp_str[PATH_MAX];

void check_bluetooth_device_type(char *rfkill_device_dir_path) {

    FILE *fp;
    char full_path[PATH_MAX];
    size_t path_len = strlen(rfkill_device_dir_path);
    size_t type_file_len = strlen(RFKILL_DEV_TYPE_FILE_NAME);

    /* Check if we have enough space for the concatenation */
    if (path_len + type_file_len >= PATH_MAX) {
        fprintf(stderr, "Path too long for rfkill device type file\n");
        exit(1);
    }

    /* Create a copy of the path to avoid modifying the original */
    strncpy(full_path, rfkill_device_dir_path, PATH_MAX - 1);
    full_path[PATH_MAX - 1] = '\0';
    strncat(full_path, RFKILL_DEV_TYPE_FILE_NAME, PATH_MAX - path_len - 1);

    if((fp = fopen(full_path, "r")) == NULL) {
        perror("fopen() error!");
        exit(1);
    }

    /* Use fgets with bounds checking instead of fscanf */
    if (fgets(bluetooth_temp_str, PATH_MAX, fp) == NULL) {
        fprintf(stderr, "Failed to read device type\n");
        fclose(fp);
        exit(1);
    }

    /* Remove newline if present */
    bluetooth_temp_str[strcspn(bluetooth_temp_str, "\n")] = '\0';

    if((fclose(fp)) == EOF) {
        perror("fclose() error!");
        exit(1);
    }
}

void find_bluetooth_rfkill_device(char *rfkill_device) {

    DIR *dirp;
    struct dirent *dir = NULL;
    char rfkill_device_dir_path[PATH_MAX];
    size_t rfkill_dir_len = strlen(RFKILL_DIR);

    /* Initialize the output parameter */
    rfkill_device[0] = '\0';

    if((dirp = opendir(RFKILL_DIR)) == NULL) {
        perror("opendir() failed!");
        exit(1);
    }

    while(1) {

        /* Use strncpy with bounds checking */
        strncpy(rfkill_device_dir_path, RFKILL_DIR, PATH_MAX - 1);
        rfkill_device_dir_path[PATH_MAX - 1] = '\0';

        if((dir = readdir(dirp)) == NULL) {
            if(errno) {
                perror("readdir() failed!");
                exit(1);
            } else {
                break;
            }
        } else {
            if(dir->d_name[0] == '.') {
                continue;
            } else {
                size_t current_len = strlen(rfkill_device_dir_path);
                size_t name_len = strlen(dir->d_name);
                
                /* Check if we have enough space for the concatenation */
                if (current_len + 1 + name_len >= PATH_MAX) {
                    fprintf(stderr, "Path too long for rfkill device directory\n");
                    continue;
                }
                
                strncat(rfkill_device_dir_path, dir->d_name, PATH_MAX - current_len - 1);
                
                check_bluetooth_device_type(rfkill_device_dir_path);

                if (!(strcmp(bluetooth_temp_str, RFKILL_DEV_TYPE_NAME))) {
                    strncpy(rfkill_device, dir->d_name, BLUETOOTH_RFKILL_DEV_NAME_LEN - 1);
                    rfkill_device[BLUETOOTH_RFKILL_DEV_NAME_LEN - 1] = '\0';
                    break;
                } else {
                    continue;
                }
            }
        }
    }

    if((closedir(dirp)) == -1) {
        perror("closedir() failed!");
        exit(1);
    }
}

short bluetooth_is_enabled(char *rfkill_device) {

    FILE *fp;
    short state = 0;
    size_t rfkill_dir_len = strlen(RFKILL_DIR);
    size_t device_len = strlen(rfkill_device);
    size_t state_file_len = strlen(RFKILL_DEV_STATE_FILE_NAME);


    /* Check if we have enough space for the full path */
    if (rfkill_dir_len + 1 + device_len + state_file_len >= PATH_MAX) {
        fprintf(stderr, "Path too long for rfkill device state file\n");
        exit(1);
    }

    strncpy(bluetooth_temp_str, RFKILL_DIR, PATH_MAX - 1);
    bluetooth_temp_str[PATH_MAX - 1] = '\0';
    
    strncat(bluetooth_temp_str, rfkill_device, PATH_MAX - rfkill_dir_len - 1);
    strncat(bluetooth_temp_str, RFKILL_DEV_STATE_FILE_NAME, 
            PATH_MAX - rfkill_dir_len - device_len - 1);

    if((fp = fopen(bluetooth_temp_str, "r")) == NULL) {
        perror("fopen() error!");
        exit(1);
    }

    if (fscanf(fp, "%hd", &state) != 1) {
        fprintf(stderr, "Failed to read bluetooth state\n");
        fclose(fp);
        exit(1);
    }

    if((fclose(fp)) == EOF) {
        perror("fclose() error!");
        exit(1);
    }

    return(state);
}

short bluetooth_is_connected(void) {

    char device_path[PATH_MAX];
    DIR *dirp;
    struct dirent *dir = NULL;
    const char *bluetooth_class_dir = "/sys/class/bluetooth/";
    size_t bluetooth_dir_len = strlen(bluetooth_class_dir);

    /* Check if bluetooth controller is powered on */
    if((dirp = opendir(bluetooth_class_dir)) == NULL) {
        return 0;
    }

    /* Look for connected devices by checking /sys/class/bluetooth/hci0/device/ */
    while((dir = readdir(dirp)) != NULL) {
        if(dir->d_name[0] == '.') {
            continue;
        }
        
        /* Check if this is a bluetooth adapter */
        if(strncmp(dir->d_name, "hci", 3) == 0) {
            size_t name_len = strlen(dir->d_name);
            const char *device_suffix = "/device/";
            size_t suffix_len = strlen(device_suffix);
            
            /* Check if we have enough space for the full path */
            if (bluetooth_dir_len + name_len + suffix_len >= PATH_MAX) {
                continue;
            }
            
            strncpy(device_path, bluetooth_class_dir, PATH_MAX - 1);
            device_path[PATH_MAX - 1] = '\0';
            
            strncat(device_path, dir->d_name, PATH_MAX - bluetooth_dir_len - 1);
            strncat(device_path, device_suffix, PATH_MAX - bluetooth_dir_len - name_len - 1);
            
            /* Check if there are connected devices */
            if(access(device_path, F_OK) == 0) {
                closedir(dirp);
                return 1;
            }
        }
    }

    closedir(dirp);
    return 0;
}

void get_connected_bluetooth_device_name(char *device_name) {

    FILE *fp;
    char line[256];
    char device_path[PATH_MAX];
    DIR *dirp;
    struct dirent *dir = NULL;
    const char *bluetooth_class_dir = "/sys/class/bluetooth/";
    size_t bluetooth_dir_len = strlen(bluetooth_class_dir);

    /* Initialize device name safely */
    strncpy(device_name, "Unknown", BLUETOOTH_DEVICE_NAME_LEN - 1);
    device_name[BLUETOOTH_DEVICE_NAME_LEN - 1] = '\0';

    /* Try to get device name from bluetoothctl or other methods */
    if((fp = popen("bluetoothctl info 2>/dev/null | grep 'Name:' | head -1 | cut -d' ' -f2-", "r")) != NULL) {
        if(fgets(device_name, BLUETOOTH_DEVICE_NAME_LEN, fp) != NULL) {
            /* Remove newline if present */
            device_name[strcspn(device_name, "\n")] = '\0';
            pclose(fp);
            return;
        }
        pclose(fp);
    }

    /* Fallback: try to read from /sys/class/bluetooth/ */
    if((dirp = opendir(bluetooth_class_dir)) == NULL) {
        return;
    }

    while((dir = readdir(dirp)) != NULL) {
        if(dir->d_name[0] == '.') {
            continue;
        }
        
        if(strncmp(dir->d_name, "hci", 3) == 0) {
            size_t name_len = strlen(dir->d_name);
            const char *device_suffix = "/device/";
            const char *uevent_suffix = "uevent";
            size_t suffix_len = strlen(device_suffix);
            size_t uevent_len = strlen(uevent_suffix);
            
            /* Check if we have enough space for the full path */
            if (bluetooth_dir_len + name_len + suffix_len + uevent_len >= PATH_MAX) {
                continue;
            }
            
            strncpy(device_path, bluetooth_class_dir, PATH_MAX - 1);
            device_path[PATH_MAX - 1] = '\0';
            
            strncat(device_path, dir->d_name, PATH_MAX - bluetooth_dir_len - 1);
            strncat(device_path, device_suffix, PATH_MAX - bluetooth_dir_len - name_len - 1);
            
            /* Try to find device name in sysfs */
            if(access(device_path, F_OK) == 0) {
                /* Look for device name in various sysfs files */
                strncat(device_path, uevent_suffix, 
                        PATH_MAX - bluetooth_dir_len - name_len - suffix_len - 1);
                
                if((fp = fopen(device_path, "r")) != NULL) {
                    while(fgets(line, sizeof(line), fp)) {
                        if(strncmp(line, "DEVNAME=", 8) == 0) {
                            size_t name_start = 8;
                            size_t name_len_available = BLUETOOTH_DEVICE_NAME_LEN - 1;
                            
                            strncpy(device_name, line + name_start, name_len_available);
                            device_name[name_len_available] = '\0';
                            
                            /* Remove newline if present */
                            device_name[strcspn(device_name, "\n")] = '\0';
                            fclose(fp);
                            closedir(dirp);
                            return;
                        }
                    }
                    fclose(fp);
                }
            }
        }
    }

    closedir(dirp);
}
