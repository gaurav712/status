/*
 * Simple statusbar showing battery percentage, date, time, wifi-status and network upload and download speed
 *
 * Copyright (c) 2019 Gaurav Kumar Yadav <gaurav712@protonmail.com>
 * for license and copyright information, see the LICENSE file distributed with this source
 *
 * This program doesn't perform any error checks for speed
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

/* function to copy num elements from n of a string to another */
void strNumFromNCpy(char *dest, char *src, short n, short num);

int main(void) {

    unsigned long up1, up2, down1, down2;
    FILE *up, *down;
    char bat_stat[12], time_str[10], date_str[11], wlan_stat;
    unsigned short bat_cap;
    time_t current_time;

    /* BATTERY STATUS */
    up = fopen("/sys/class/power_supply/BAT1/status", "r");
    fscanf(up, "%s", bat_stat);
    fclose(up);


    /* BATTERY PERCENTAGE */
    up = fopen("/sys/class/power_supply/BAT1/capacity", "r");
    fscanf(up, "%hd", &bat_cap);
    fclose(up);

    /* DATE */
    current_time = time(NULL);
    strNumFromNCpy(date_str, ctime(&current_time), 0, 10);

    /* TIME */
    strNumFromNCpy(time_str, ctime(&current_time), 11, 8);

    printf("%d%%(%s) :: %s :: %s", bat_cap, bat_stat, date_str, time_str);

    /* RFKILL */
    up = fopen("/sys/class/rfkill/rfkill0/state", "r");
    fscanf(up, "%c", &wlan_stat);
    fclose(up);

    if(wlan_stat == '1') {

        /* WIFI STATUS(connected or not) */
        up = fopen("/sys/class/net/wlp3s0/operstate", "r");
        fscanf(up, "%c", &wlan_stat);
        fclose(up);

        if(wlan_stat == 'u') {

            /* UPLOAD AND DOWNLOAD SPEED */
            up = fopen("/sys/class/net/wlp3s0/statistics/tx_bytes", "r");
            down = fopen("/sys/class/net/wlp3s0/statistics/rx_bytes", "r");
            fscanf(up, "%ld", &up1);
            fscanf(down, "%ld", &down1);
            sleep(1);

            /* Moving the pointer again to the begining */
            rewind(up);
            rewind(down);
            fscanf(up, "%ld", &up2);
            fscanf(down, "%ld", &down2);
            fclose(up);
            fclose(down);
            printf(" :: %.2fkb/s %.2fkb/s", (down2 - down1)/1024.0, (up2 - up1)/1024.0);

        } else {
            printf(" :: Disconnected");
            sleep(1);
        }
    } else {
        printf(" :: WLAN_OFF");
        sleep(1);
    }


    return 0;
}

void
strNumFromNCpy(char *dest, char *src, short n, short num){

    short count = 0;
    for(; count < num; count++){
        dest[count] = src[n];
        n++;
    }
    dest[count] = '\0';
}

