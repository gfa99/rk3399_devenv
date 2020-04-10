#ifndef LIST_NETWORK_H
#define LIST_NETWORK_H

#include "../utility/list.h"

#define SSID_LEN_MAX	128	//32	/xe8/xa5
#define PW_LEN_MAX	128

struct list_network_s {
	struct list_head  i_list;
	int netId;
	char ssid[SSID_LEN_MAX+4];
	char bssid[32];
	char flags[64];
};

extern int addNetworkList(char * line, struct list_head * list);
extern int updateNetworkList(char * networks_list, struct list_head * list);
extern void networkListInit(struct list_head * list);
extern void clearNetworkList(struct list_head * list);
#endif //LIST_NETWORK_H
