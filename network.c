#include "network.h"

#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/wireless.h>
#include <ifaddrs.h>
#include <time.h>

struct timespec wait_duration = {0, 500000000L};

#define RFKILL_DIR  "/sys/class/rfkill/"
#define RFKILL_DEV_TYPE_FILE_NAME   "/type"
#define RFKILL_DEV_TYPE_NAME   "wlan"
#define RFKILL_DEV_STATE_FILE_NAME  "/state"
#define INTERFACE_NAME_LEN  20
#define NET_DEVICES_DIR "/sys/class/net/"
#define NET_DEVICE_STATE_FILE_NAME  "/operstate"
#define NET_DEVICE_UP_STATE_KEYWORD "up"
#define NET_DEVICE_UP_BYTES_FILE    "/statistics/tx_bytes"
#define NET_DEVICE_DOWN_BYTES_FILE  "/statistics/rx_bytes"

char temp_str[PATH_MAX];
char interface_name[INTERFACE_NAME_LEN];

void find_rfkill_device(char *rfkill_device) {

    DIR *dirp;
    struct dirent *dir = NULL;
    char rfkill_device_dir_path[PATH_MAX];

    if((dirp = opendir(RFKILL_DIR)) == NULL) {
        perror("opendir() failed!");
        exit(1);
    }

    while(1) {

        strcpy(rfkill_device_dir_path, RFKILL_DIR);

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
                strcat(rfkill_device_dir_path, "/");
                strcat(rfkill_device_dir_path, dir->d_name);
                check_device_type(rfkill_device_dir_path);

                if (!(strcmp(temp_str, RFKILL_DEV_TYPE_NAME))) {
                    strcpy(rfkill_device, dir->d_name);
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

void check_device_type(char *rfkill_device_dir_path) {

    FILE *fp;

    strcat(rfkill_device_dir_path, RFKILL_DEV_TYPE_FILE_NAME);

    if((fp = fopen(rfkill_device_dir_path, "r")) == NULL) {
        perror("fopen() error!");
        exit(1);
    }

    fscanf(fp, "%s", temp_str);

    if((fclose(fp)) == EOF) {
        perror("fclose() error!");
        exit(1);
    }
}

short network_is_enabled(char *rfkill_device) {

    FILE *fp;
    short state = 0;

    strcpy(temp_str, RFKILL_DIR);
    strcat(temp_str, "/");
    strcat(temp_str, rfkill_device);
    strcat(temp_str, RFKILL_DEV_STATE_FILE_NAME);

    if((fp = fopen(temp_str, "r")) == NULL) {
        perror("fopen() error!");
        exit(1);
    }

    fscanf(fp, "%hd", &state);

    if((fclose(fp)) == EOF) {
        perror("fclose() error!");
        exit(1);
    }

    return(state);
}

short network_is_connected(void) {

    FILE *fp;
    char dev_state[10];

    get_wireless_network_interface_name();

    strcpy(temp_str, NET_DEVICES_DIR);
    strcat(temp_str, interface_name);
    strcat(temp_str, NET_DEVICE_STATE_FILE_NAME);

    if((fp = fopen(temp_str, "r")) == NULL) {
        perror("fopen() error!");
        exit(1);
    }

    fscanf(fp, "%s", dev_state);

    if((fclose(fp)) == EOF) {
        perror("fclose() error!");
        exit(1);
    }

    if(!strcmp(dev_state, NET_DEVICE_UP_STATE_KEYWORD))
        return 1;

    return 0;
}

void get_wireless_network_interface_name(void) {

    struct ifaddrs *ifa;

    if (getifaddrs(&ifa) == -1) {
        perror("getifaddrs() failed!");
        exit(1);
    }

    while(ifa->ifa_next != NULL) {
        if(interface_is_wireless(ifa->ifa_name)) {
            strcpy(interface_name, ifa->ifa_name);
            return;
        }

        ifa = ifa->ifa_next;
    }

    fprintf(stderr, "no wireless interfaces!\n");
    exit(1);
}

/*
    This is a pretty neat hack by a guy named "Edu Felipe" to check if an interface is wireless.
    Check it out at https://gist.github.com/edufelipe/6108057
*/

short interface_is_wireless(const char *device) {

    int sock = -1;
    struct iwreq iw;
    char protocol[IFNAMSIZ]  = {0};

    memset(&iw, 0, sizeof(iw));
    strcpy(iw.ifr_name, device);

    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket() failed!");
        exit(1);
    }

    if (ioctl(sock, SIOCGIWNAME, &iw) != -1) {
        if (protocol)
            strcpy(protocol, iw.u.name);
        close(sock);
        return 1;
    }

    return 0;
}


void get_bytes_transferred(float *down_bytes, float *up_bytes) {

    FILE *up, *down;
    unsigned long up1, up2, down1, down2;
    char up_path[PATH_MAX], down_path[PATH_MAX];

    strcpy(up_path, NET_DEVICES_DIR);
    strcat(up_path, interface_name);
    strcpy(down_path, up_path);

    strcat(down_path, NET_DEVICE_DOWN_BYTES_FILE);
    strcat(up_path, NET_DEVICE_UP_BYTES_FILE);

    if((down = fopen(down_path, "r")) == NULL) {
        perror("fopen() failed!");
        exit(1);
    }

    if((up = fopen(up_path, "r")) == NULL) {
        perror("fopen() failed!");
        exit(1);
    }

    fscanf(down, "%ld", &down1);
    fscanf(up, "%ld", &up1);

    nanosleep(&wait_duration, NULL);

    rewind(down);
    rewind(up);

    fscanf(down, "%ld", &down2);
    fscanf(up, "%ld", &up2);

    *down_bytes = (down2 - down1) / 512.0;
    *up_bytes = (up2 - up1) / 512.0;

    if((fclose(down)) == EOF) {
        perror("fclose() error!");
        exit(1);
    }

    if((fclose(up)) == EOF) {
        perror("fclose() error!");
        exit(1);
    }
}
