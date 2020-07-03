#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <poll.h>
#include <sys/utsname.h>
#include <pthread.h>

#include "lollipop_wifiScanAp.h"
#include "../utility/string_ext.h"
#include "../utility/common_web.h"

#define DBG 0

#if 0
#define BSSID_STR       "bssid="
#define FREQ_STR        "freq="
#define LEVEL_STR       "level="
#define TSF_STR         "tsf="
#define FLAGS_STR       "flags="
#define SSID_STR        "ssid="
#define DELIMITER_STR   "===="
#endif

void clearScanApList(struct list_head * list) {
        struct list_head *pList;
        struct list_head *pListTemp;

        list_for_each_safe(pList, pListTemp, list) {
                struct scan_result_s *ap = list_entry(pList, struct scan_result_s, i_list);
                if(DBG) ALOGD("delete ap list, ssid=%s", ap->ssid);
                list_del(pList);
                if(ap != NULL) free(ap);
        }
}

#if 0
/**
 * Format:
 * bssid=68:7f:76:d7:1a:6e
 * freq=2412
 * level=-44
 * tsf=1344626243700342
 * flags=[WPA2-PSK-CCMP][WPS][ESS]
 * ssid=zfdy
 * ====
 * bssid=68:5f:74:d7:1a:6f
 * freq=5180
 * level=-73
 * tsf=1344626243700373
 * flags=[WPA2-PSK-CCMP][WPS][ESS]
 * ssid=zuby
 * ====
 */
int addScanAp(char * line) {
	char bssid[SSID_LENGTH] = {0};
	char ssid[SSID_LENGTH] = {0};
	int level = 0;
	int freq = 0;
	int tsf = 0;
	char flags[256] = {0};

	if(line == NULL) return -1;

	if (str_startsWith(line, BSSID_STR)) {
		strcpy(bssid, line+strlen(BSSID_STR));
	} else if (str_startsWith(line, FREQ_STR)) {
		freq = atoi(line+strlen(FREQ_STR));
	} else if (str_startsWith(line, LEVEL_STR)) {
		level = atoi(line+strlen(LEVEL_STR));
		/* some implementations avoid negative values by adding 256
		* so we need to adjust for that here.
		*/
		if (level > 0) level -= 256;
	} else if (str_startsWith(line, TSF_STR)) {
		tsf = atoi(line+strlen(TSF_STR));
	} else if (str_startsWith(line, FLAGS_STR)) {
		strcpy(flags, line+strlen(FLAGS_STR));
	} else if (str_startsWith(line, SSID_STR)) {
		strcpy(ssid, line+strlen(SSID_STR));
	} else if (str_startsWith(line, DELIMITER_STR)) {
		if (strlen(bssid)>0) {
			struct scan_result_s result;
			strcpy(result.bssid, bssid);
			strcpy(result.capabilities, flags);
			result.frequency = freq;
			result.level = level;
			strcpy(result.ssid, ssid);
			result.timestamp = tsf;
			list_add_tail(&result.i_list, &gScanApList);
		} else {
			strcpy(bssid, "");
			strcpy(ssid, "");
			level = 0;
			freq = 0;
			tsf = 0;
			strcpy(flags,"");
		}
	}

	return 0;
}
#endif

/*
Using interface 'wlan0'
bssid / frequency / signal level / flags / ssid
00:23:69:24:38:73       2412    -36     [WPA-PSK-TKIP+CCMP][WPA2-PSK-TKIP+CCMP][WPS][ESS]       linksys
20:dc:e6:9a:60:46       2462    -41     [WPA-PSK-CCMP][WPA2-PSK-CCMP][WPS][ESS] rkmobile
00:1f:a3:d0:97:79       2437    -42     [WPA-PSK-CCMP][WPA2-PSK-CCMP][ESS]      JOYE-DUAL
ec:88:8f:7d:5e:98       2437    -45     [WPA-PSK-CCMP][WPS][ESS]        rockchip
c4:ca:d9:1c:ac:72       2462    -47     [WPA2-EAP-CCMP][ESS]    CMCC-AUTO
*/
int parseScanResult(char * line, struct scan_result_s * ap) {
	char * p;
	char *last;

	if(line == NULL) return -1;
	if(ap == NULL) return -1;

	//if(0 == strncmp(str, "Using", strlen("Using"))) return -1;
	if(0 == strncmp(line, "bssid", strlen("bssid"))) return -1;

	p = strtok_r(line, "\t", &last);	
	if(DBG) ALOGD("p1---%s", p);
	if(p != NULL) strcpy(ap->bssid, p);

	p = strtok_r(NULL, "\t", &last);
	if(DBG) ALOGD("p2---%s", p);
	if(p != NULL) ap->freq = atoi(p);

	p = strtok_r(NULL, "\t", &last);
	if(DBG) ALOGD("p3---%s", p);
	if(p != NULL) ap->level = atoi(p);
	/* some implementations avoid negative values by adding 256
		* so we need to adjust for that here.
		*/
	if (ap->level > 0) ap->level -= 256;

	p = strtok_r(NULL, "\t", &last);
	if(DBG) ALOGD("p4---%s", p);
	if(p != NULL) strcpy(ap->flags, p);

	p = strtok_r(NULL, "\t", &last);
	if(DBG) ALOGD("p5---%s", p);
#if 1
	if (p != NULL) {
		char ssid1[SSID_LEN_MAX+4] = {0};
		int m = 0;
		int n = 0;
		while(1) {
			if (m > strlen(p)) break;

			if (((p[m] == '\\') && (p[m+1] == '\\')) || 
				((p[m] == '\\') && (p[m+1] == '\"'))) {
				ssid1[n++] = p[m+1];
				m++;
				m++;
			} else {
				ssid1[n++] = p[m++];
			}
		}
		strcpy(ap->ssid, ssid1);
	}
#else
	if(p != NULL) strcpy(ap->ssid, p);
#endif
	return 0;
}


int addScanAp(char * line, struct list_head * list) {
	if (line == NULL) return -1;
        if (list == NULL) return -1;

        if(DBG) ALOGD("addScanAp: %s", line);

        if(str_startsWith(line, "bssid")) return -1;

        struct scan_result_s * ap = (struct scan_result_s *)malloc(sizeof(*ap));
        if(parseScanResult(line, ap)) {
                free(ap);
                ALOGD("parseScanResult failed");
                return -1;
        }

        list_add_tail(&ap->i_list, list);

        return 0;
}

int updateScanApList(char * scan_result, struct list_head * list) {
        int i=0;
        char * p;
        char line[512];
	char * str_begin = scan_result;
	int str_len = strlen(scan_result);
	char *last;

        if (list == NULL) return -1;

        clearScanApList(list);

        if(DBG) ALOGD("updateScanApList:%s\n", scan_result);

        p = scan_result;
        p = strtok_r(p, "\n", &last);
        while(p) {
                if(DBG) ALOGD("---p: %s", p);
                strcpy(line, p);
                addScanAp(line, list);
		//ALOGD("<<<<<<<<<<p=0x%x", p);
		//ALOGD("p len=%d", strlen(p));
                p = p + strlen(p) +1;
		if (p >= str_begin+str_len) break;
		//ALOGD("p=0x%x>>>>>>>>>>>", p);
                p = strtok_r(p, "\n", &last);
        }

        return 0;
}

void scanApListInit(struct list_head * list) {
        INIT_LIST_HEAD(list);
}
