#ifndef OPERATE_H
#define OPERATE_H

#define RSSI_LEVELS 	6

extern int calculateSignalLevel(int rssi, int numLevels);
extern int wpa_cli_cmd(char * iface, char * cmd, char * ret, int len);
extern int wpa_cli_cmd2(char * iface, char * cmd);
extern int getNetworkList(char * iface, struct list_head * list);
extern int getConnectedNetId(char * iface, char * ssid);
extern int getNetIdBySsid(char * iface, char * ssid);
extern int scan(char * iface, struct list_head * list);
extern int isOpenWifi(char * flags);
extern int connect_network(char * iface, char * ssid, char * pw, int isAscii, char * flags, int isNew);
extern int disconnect(char * iface, char * ssid);
extern int forget(char * iface, char * ssid);
/*
return:
0: not connected
1: saved
2: connected
*/
extern int getNetworkState(char * iface, char * ssid);

extern int get_wifi_status(char * status, int size);
extern int get_dhcp_status(char * status, int size);
#endif //OPERATE_H
