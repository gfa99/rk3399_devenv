#ifndef __COMMON_WEB_H
#define __COMMON_WEB_H

#include <stdio.h>

#if 1
#define ALOGD(fmt, args...) do { \
    FILE *fp = fopen("/dev/console", "w"); \
    if (fp) { \
        fprintf(fp, "\t%s %d", __FUNCTION__, __LINE__); \
        fprintf(fp, fmt, ## args); \
        fprintf(fp, "\n"); \
        fclose(fp); \
    } \
} while (0)

#define ALOGE ALOGD

#else
#define ALOGD(fmt, arg...)
#define ALOGE(fmt, arg...)
#endif

#define WLAN_IFACE	"wlan0"
#define P2P_IFACE	"p2p0"

#define bool char

bool Wifi_Connect(char *ssid, char *password, char *security, bool hide);
#endif
