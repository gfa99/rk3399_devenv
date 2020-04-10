#ifndef LOLLIPOP_SCAN_AP_LIST_H
#define LOLLIPOP_SCAN_AP_LIST_H

#include "list_network.h"
#include "../utility/list.h"

#define SSID_LEN_MAX	128	//32	/xe8/xa5
struct scan_result_s {
	struct list_head i_list;
	char ssid[SSID_LEN_MAX+4];
	char bssid[32];
	char flags[256];
	int level;
	int freq;
	int timestamp;
};

extern void updateScanResults(char * scanResults, struct list_head * list);
extern void scanApListInit(struct list_head * list);
extern void clearScanApList(struct list_head * list);
extern int updateScanApList(char * scan_result, struct list_head * list);
//extern int addScanAp(char * line, struct list_head * list);
#endif //LOLLIPOP_SCAN_AP_LIST_H
