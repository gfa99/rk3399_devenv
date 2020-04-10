#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/wireless.h>

//#include <openssl/evp.h>
//#include <openssl/sha.h>


#include "lollipop_softap.h"
#include "list_network.h"

#define HOSTAPD_CONF_FILE "/data/misc/wifi/hostapd.conf"
#define NEED_SOCK 0

static int mPid;

int softap_init(void) {
    mPid = 0;
#if NEED_SOCK
    mSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (mSock < 0)
        ALOGE("Failed to open socket");
#endif
    return 0;
}

void softap_deinit(void) {
#if NEED_SOCK
    if (mSock >= 0)
        close(mSock);
#endif
}

int startSoftap(void) {
    pid_t pid = 1;
    int ret = 0;

    if (mPid) {
        ALOGE("Softap already started");
        return 0;
    }
#if NEED_SOCK
    if (mSock < 0) {
        ALOGE("Softap startap - failed to open socket");
        return -1;
    }
#endif
    if ((pid = fork()) < 0) {
        ALOGE("fork failed (%s)", strerror(errno));
        return -1;
    }

    if (!pid) {
		ALOGD("start rtl_hostapd ...");
		system("softapDemo Rockchip_WEB_888");
        ALOGE("Should never get here!");
        return -1;
    } else {
        mPid = pid;
        ALOGD("Softap startap - Ok");
        usleep(AP_BSS_START_DELAY);
    }
    return ret;

}

int stopSoftap(void) {

    if (mPid == 0) {
        ALOGE("Softap already stopped");
        return 0;
    }

    ALOGD("Stopping Softap service");
    kill(mPid, SIGTERM);
    waitpid(mPid, NULL, 0);

#if NEED_SOCK
    if (mSock < 0) {
        ALOGE("Softap stopap - failed to open socket");
        return -1;
    }
#endif
    mPid = 0;
    ALOGD("Softap service stopped");
    usleep(AP_BSS_STOP_DELAY);
    return 0;
}

int isSoftapStarted(void) {
    //return (mPid != 0 ? true : false);
    return (mPid != 0 ? 1 : 0);
}

/*
 * Arguments:
 *      argv[2] - wlan interface
 *      argv[3] - SSID
 *	argv[4] - Security
 *	argv[5] - Key
 *	argv[6] - Channel
 *	argv[7] - Preamble
 *	argv[8] - Max SCB
 */
#define HT_CAPAB "ht_capab=[SHORT-GI-20][SHORT-GI-40][HT40]\n"
int setSoftap(int argc, char *argv[]) {
#if 0
    char psk_str[2*SHA256_DIGEST_LENGTH+1];
    int ret = 0, i = 0, fd;
    char *ssid, *iface;

#if NEED_SOCK
    if (mSock < 0) {
        ALOGE("Softap set - failed to open socket");
        return -1;
    }
#endif
    if (argc < 4) {
        ALOGE("Softap set - missing arguments");
        return -1;
    }

    iface = argv[2];

    char *wbuf = NULL;
    char *fbuf = NULL;

    if (argc > 3) {
        ssid = argv[3];
    } else {
        ssid = (char *)"AndroidAP";
    }

    asprintf(&wbuf, "interface=%s\ndriver=nl80211\nctrl_interface="
            "/data/misc/wifi/hostapd\nssid=%s\nchannel=6\nhw_mode=g\nieee80211n=1\n"HT_CAPAB,
            iface, ssid);

    if (argc > 4) {
        if (!strcmp(argv[4], "wpa-psk")) {
            generatePsk(ssid, argv[5], psk_str);
            asprintf(&fbuf, "%swpa=1\nwpa_pairwise=TKIP CCMP\nwpa_psk=%s\n", wbuf, psk_str);
        } else if (!strcmp(argv[4], "wpa2-psk")) {
            generatePsk(ssid, argv[5], psk_str);
            asprintf(&fbuf, "%swpa=2\nrsn_pairwise=CCMP\nwpa_psk=%s\n", wbuf, psk_str);
        } else if (!strcmp(argv[4], "open")) {
            asprintf(&fbuf, "%s", wbuf);
        }
    } else {
        asprintf(&fbuf, "%s", wbuf);
    }

    fd = open(HOSTAPD_CONF_FILE, O_CREAT | O_TRUNC | O_WRONLY | O_NOFOLLOW, 0660);
    if (fd < 0) {
        ALOGE("Cannot update \"%s\": %s", HOSTAPD_CONF_FILE, strerror(errno));
        free(wbuf);
        free(fbuf);
        return -1;
    }
    if (write(fd, fbuf, strlen(fbuf)) < 0) {
        ALOGE("Cannot write to \"%s\": %s", HOSTAPD_CONF_FILE, strerror(errno));
        ret = -1;
    }
    free(wbuf);
    free(fbuf);

    /* Note: apparently open can fail to set permissions correctly at times */
    if (fchmod(fd, 0660) < 0) {
        ALOGE("Error changing permissions of %s to 0660: %s",
                HOSTAPD_CONF_FILE, strerror(errno));
        close(fd);
        unlink(HOSTAPD_CONF_FILE);
        return -1;
    }

    if (fchown(fd, AID_SYSTEM, AID_WIFI) < 0) {
        ALOGE("Error changing group ownership of %s to %d: %s",
                HOSTAPD_CONF_FILE, AID_WIFI, strerror(errno));
        close(fd);
        unlink(HOSTAPD_CONF_FILE);
        return -1;
    }

    close(fd);
    return ret;
#endif
}

#if 0
void generatePsk(char *ssid, char *passphrase, char *psk_str) {
    unsigned char psk[SHA256_DIGEST_LENGTH];
    int j;
    // Use the PKCS#5 PBKDF2 with 4096 iterations
    PKCS5_PBKDF2_HMAC_SHA1(passphrase, strlen(passphrase),
            /*reinterpret_cast<const unsigned char *>*/ssid, strlen(ssid),
            4096, SHA256_DIGEST_LENGTH, psk);
    for (j=0; j < SHA256_DIGEST_LENGTH; j++) {
        sprintf(&psk_str[j<<1], "%02x", psk[j]);
    }
    psk_str[j<<1] = '\0';
}


/*
 * Arguments:
 *	argv[2] - interface name
 *	argv[3] - AP or STA
 */
int fwReloadSoftap(int argc, char *argv[])
{
    int ret, i = 0;
    char *iface;
    char *fwpath;
#if NEED_SOCK
    if (mSock < 0) {
        ALOGE("Softap fwrealod - failed to open socket");
        return -1;
    }
#endif
    if (argc < 4) {
        ALOGE("Softap fwreload - missing arguments");
        return -1;
    }
	
	ALOGE("Wifi chip is from realtek, not need to change fw!");
	return 0;

    iface = argv[2];

    if (strcmp(argv[3], "AP") == 0) {
        fwpath = (char *)wifi_get_fw_path(WIFI_GET_FW_PATH_AP);
    } else if (strcmp(argv[3], "P2P") == 0) {
        fwpath = (char *)wifi_get_fw_path(WIFI_GET_FW_PATH_P2P);
    } else {
        fwpath = (char *)wifi_get_fw_path(WIFI_GET_FW_PATH_STA);
    }
    if (!fwpath)
        return -1;
    ret = wifi_change_fw_path((const char *)fwpath);
    if (ret) {
        ALOGE("Softap fwReload - failed: %d", ret);
    }
    else {
        ALOGD("Softap fwReload - Ok");
    }
    return ret;
}

int clientsSoftap(char **retbuf)
{
    /* TODO: implement over hostapd */
    return 0;
}
#endif

