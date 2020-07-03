#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "../utility/string_ext.h"
#include "../utility/list.h"
#include "list_network.h"
#include "../utility/common_web.h"

#define DBG 0

/*
network id / ssid / bssid / flags
1       DIRECT-r9-LOLLIPOP-89178        02:2f:05:00:80:a9       [DISABLED][P2P-PERSISTENT]
3       DIRECT-bX-LOLLIPOP-89178        02:2f:05:00:80:a9       [DISABLED][P2P-PERSISTENT]
4       DIRECT-bX-LOLLIPOP-89178        any     [CURRENT]
*/

void clearNetworkList(struct list_head * list) {
	struct list_head *pList;
	struct list_head *pListTemp;

	list_for_each_safe(pList, pListTemp, list) {
		struct list_network_s *nw = list_entry(pList, struct list_network_s, i_list);
		if(DBG) ALOGD("delete newtork list, ssid=%s", nw->ssid);
		list_del(pList);
		if(nw != NULL) free(nw);
	}
}

static int parseNetworkList(char * listStr, struct list_network_s * nw) {
	char * p;
	char *last;

	if (listStr == NULL) return -1;
	if (nw == NULL) return -1;
	if (str_startsWith(listStr, "network")) return -1;

	if(DBG) ALOGD("---------------------");
	if(DBG) ALOGD("parse: %s", listStr);

	p = strtok_r(listStr, "\t", &last);
	if(p == NULL) return -1;
	nw->netId = atoi(p); 
	if(DBG) ALOGD("netId=%s", p);

	p = strtok_r(NULL, "\t", &last);
	if(p == NULL) return -1;
#if 1
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
	strcpy(nw->ssid, ssid1);
	if(DBG)ALOGD("ssid=%s", ssid1);
#else
	strcpy(nw->ssid,p);
	if(DBG)ALOGD("ssid=%s", p);
#endif
	p = strtok_r(NULL, "\t", &last);
	if(p == NULL) return -1;
	strcpy(nw->bssid,p);
	if(DBG) ALOGD("bssid=%s", p);

	p = strtok_r(NULL, "\t", &last);
	if (p != NULL) {
		strcpy(nw->flags,p);
		if(DBG) ALOGD("flags=%s", p);
	}
	if(DBG) ALOGD("---------------------");
	return 0;
}

int addNetworkList(char * line, struct list_head * list) {
	if (list == NULL) return -1;

	//if(DBG) ALOGD("addNetworkList: %s", line);

	if(str_startsWith(line, "network")) return -1;

	struct list_network_s * nw = (struct list_network_s *)malloc(sizeof(*nw));
	if(parseNetworkList(line, nw)) {
		free(nw);
		ALOGD("parseNetworkList failed");
		return -1;
	}

	list_add_tail(&nw->i_list, list);

	return 0;
}

int updateNetworkList(char * networks_list, struct list_head * list) {
	int i=0;
	char * p;
	char * str_end = networks_list + strlen(networks_list);
	char line[512];
	char *last;

	if (list == NULL) return -1;

        clearNetworkList(list);

	if(DBG) ALOGD("updateNetworkList:%s\n", networks_list);

	p = networks_list;
	p = strtok_r(p, "\n", &last);
	while(p) {
		//if(DBG) ALOGD("---p: %s", p);
		if(str_startsWith(p, "network")) {
			p = strtok_r(NULL, "\n", &last);
			continue;
		}
		strcpy(line, p);
		addNetworkList(line, list);
		p = p + strlen(p) +1;
		if(p >= str_end) break; 
		p = strtok_r(p, "\n", &last);
	}

	return 0;
}

void networkListInit(struct list_head * list) {
	INIT_LIST_HEAD(list);
}
