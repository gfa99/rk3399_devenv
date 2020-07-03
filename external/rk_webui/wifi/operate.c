#include <unistd.h>
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

//#include "../wifi/lollipop_wifiNative.h"
//#include "../wifi/lollipop_wifiMonitor.h"
//#include "../wifi/lollipop_wifiScanAp.h"
#include "../wifi/list_network.h"
#include "../utility/list.h"
#include "../utility/string_ext.h"
//#include "../config/lollipop_config.h"
//#include "../wifi/lollipop_netTools.h"
#include "../wifi/operate.h"
#include "../utility/common_web.h"

#define DBG 1

#define CMD_BUF_LEN 4098

#define MIN_RSSI -100
/** Anything better than or equal to this will show the max bars. */
#define MAX_RSSI -55
static volatile bool wifi_wrong_key = false;

static void execute(const char cmdline[], char recv_buff[], int len)
{
	if (DBG)
		ALOGD("[AIRKISS] execute: %s", cmdline);

	FILE *stream = NULL;
	char *tmp_buff = recv_buff;

	memset(recv_buff, 0, len);

	if ((stream = popen(cmdline, "r")) != NULL) {
		while (fgets(tmp_buff, len, stream)) {
			//printf("tmp_buf[%d]: %s\n", strlen(tmp_buff), tmp_buff);
			tmp_buff += strlen(tmp_buff);
			len -= strlen(tmp_buff);
			if (len <= 1)
				break;
		}
		if (DBG)
			ALOGD("[AIRKISS] execute_r: %s", recv_buff);
		pclose(stream);
	}
}

char *www_spec_char_convers(const char *buf, char *dst)
{
	char buf_temp[strlen(buf) + 1];
	int i = 0;
	unsigned long con;

	memset(buf_temp, 0, sizeof(buf_temp));
	while(*buf != '\0') {
		if(*buf == '\\' && *(buf + 1) == 'x') {
			strcpy(buf_temp, buf);
			*buf_temp = '0';
			*(buf_temp + 4) = '\0';
			con = strtoul(buf_temp, NULL, 16);
			if (con == 0)
				con += 32;
			dst[i] = con;
			buf += 3;
		} else {
			dst[i] = *buf;
		}
		i++;
		buf++;
	}
	dst[i] = '\0';
	return dst;
}

int calculateSignalLevel(int rssi, int numLevels) {
	if (rssi <= MIN_RSSI) {
		return 0;
	} else if (rssi >= MAX_RSSI) {
		return numLevels - 1;
	} else {
		int inputRange = (MAX_RSSI - MIN_RSSI);
		int outputRange = (numLevels - 1);
		return (int)((rssi - MIN_RSSI) * outputRange / inputRange);
	}
}

int wpa_cli_cmd(char * iface, char * cmd, char * ret, int len) {
	char buf[256] = {0};
	int netId = -1;

	if (cmd == NULL) return -1;
	if (ret == NULL) return -1;

	memset(buf, 0, sizeof(buf));

	//if (!strcmp(iface, P2P_IFACE)) {
		//sprintf(buf, "/usr/sbin/wpa_cli -i %s %s", P2P_IFACE, cmd);
	//} else if (!strcmp(iface, WLAN_IFACE)) {
	sprintf(buf, "/usr/sbin/wpa_cli -i %s %s", WLAN_IFACE, cmd);
	//}

	ALOGD("======= execute xxx /usr/sbin/wpa_cli command: %s =========", buf);

	if (0 == exec_command(buf, ret, len)) {
		if (strlen(ret) <= 0) {
			ALOGE("%s failed!! no return", cmd);
			return -1;
		}

	} else {
		ALOGE("%s failed!!", cmd);
		return -1;
	}

	ALOGD("wpa_cli_cmd return %s", ret);

	return 0;
}

int wpa_cli_cmd2(char * iface, char * cmd) {
	char ret[64] = {0};

	if(wpa_cli_cmd(iface, cmd, ret, sizeof(ret))) return -1;
#ifndef RTK_WIFI_VENDOR
	if(strstr(ret, "OK") == NULL) {
#else
	if(strncmp(ret, "OK", 2)) {
#endif
		ALOGE("%s failed!! ret=%s", cmd, ret);
		return -1;
	}

	return 0;
}

int getNetworkList(char * iface, struct list_head * list) {
        char *wpa_cli_return = (char *)malloc(CMD_BUF_LEN);
        char *wpa_cli_return_conv = (char *)malloc(CMD_BUF_LEN);
	struct list_head * pList;
	int ret = -1;

	memset(wpa_cli_return, 0, CMD_BUF_LEN);
	
	if(wpa_cli_cmd(iface, "list_networks", wpa_cli_return, CMD_BUF_LEN)) {
		ALOGE("get group network list failed");
		ret = -1;
		goto out;
	}

	www_spec_char_convers(wpa_cli_return, wpa_cli_return_conv);
	updateNetworkList(wpa_cli_return_conv, list);
	ret = 0;
out:
	free(wpa_cli_return);
	free(wpa_cli_return_conv);
	return ret;
}

int getConnectedNetId(char * iface, char * ssid) {
	int ret = -1;
	struct list_head nwList;
	struct list_head * pList;

	networkListInit(&nwList);
        getNetworkList(iface, &nwList);

	list_for_each(pList, &nwList) {
                struct list_network_s * nw = list_entry(pList, struct list_network_s, i_list);
                if(nw == NULL) continue;
                if(nw->ssid == NULL) continue;

		if(!strncmp(nw->flags, "[CURRENT]", strlen("[CURRENT]"))) {
			ALOGD("%s is connected", nw->ssid);
			if (ssid) strcpy(ssid, nw->ssid);
			ret = nw->netId;
			break;
		}
	}

	clearNetworkList(&nwList);
	return ret;
}

int getNetIdBySsid(char * iface, char * ssid) {
	int ret = -1;
	struct list_head nwList;
	struct list_head * pList;

	networkListInit(&nwList);
	getNetworkList(iface, &nwList);

	list_for_each(pList, &nwList) {
                struct list_network_s * nw = list_entry(pList, struct list_network_s, i_list);
                if(nw == NULL) continue;
                if(nw->ssid == NULL) continue;

		if(!strcmp(nw->ssid, ssid)) {
			ALOGD("get %s's netId %d", nw->ssid, nw->netId);
			ret = nw->netId;
			break;
		}
	}

	clearNetworkList(&nwList);
	return ret;
}

int scan(char * iface, struct list_head * list) {
	int ret = -1;
	int tryCount = 0;
	char * wpa_cli_return = (char *)malloc(10240);
	char * wpa_cli_return_conv = (char *)malloc(10240);

	memset(wpa_cli_return, 0, 10240);

	while(1) {
		if (tryCount++ > 5) goto out;

		wpa_cli_cmd2(iface, "scan");

		sleep(2);

        	wpa_cli_cmd(iface, "scan_result", wpa_cli_return, 10240);

		if (strlen(wpa_cli_return) > strlen("bssid / frequency / signal level / flags / ssid") + 10) break;
	}

	www_spec_char_convers(wpa_cli_return, wpa_cli_return_conv);
	updateScanApList(wpa_cli_return_conv, list);

	ret = 0;
out:
	free(wpa_cli_return);
	free(wpa_cli_return_conv);
	return ret;
}

int isOpenWifi(char * flags) {
	if ((strstr(flags, "WEP") == NULL) && 
		(strstr(flags, "PSK") == NULL)) {
		return 1;
	} else {
		return 0;
	}
}

int process_string_ssid(char *in, char *out)
{
	int i, j;

	i = 0;
	j = 0;

	for (i = 0; i < strlen(in); i++) {
		out[j++] = '\\';
		out[j++] = in[i];
	}
	out[j] = '\0';
	return 1;
}

int process_string(char * in, char * out, int out_buf_len) {
	if (in == NULL) return -1;
	if (out == NULL) return -1;
	if (out_buf_len == 0) return -1;
	int ret = 0;
	int in_len = strlen(in);
	int j = 0;
	int k = 0;

#if 1
	if (out_buf_len < in_len*2) return -1; 

	while(in[j] != '\0') {
		if (((in[j] >= '0') && (in[j] <= '9')) || 
			((in[j] >= 'a') && (in[j] <= 'z')) || 
			((in[j] >= 'A') && (in[j] <= 'Z'))) {

			out[k++] = in[j++];
		}else {
			out[k++] = '\\';
			out[k++] = in[j++];
		}
	}

	out[k] = '\0';
#endif
	return strlen(out);
}

int get_wifi_status(char *status, int size) {
	char wifi_status[128];
	memset(wifi_status, 0, sizeof(wifi_status));

	exec_command("/usr/sbin/wpa_cli -i wlan0 status | grep wpa_state", wifi_status, sizeof(wifi_status));

	if (strstr(wifi_status, "DISCONNECTED"))
		strcpy(status, "disconnected");
	else if (strstr(wifi_status, "COMPLETED"))
		strcpy(status, "connected");
	else
		strcpy(status, "connecting");
	//ALOGD("wifi_status: %s", status);		
	return 0;
}

int get_dhcp_status(char * status, int size) {
	char dhcp_status[128];
	memset(dhcp_status, 0, sizeof(dhcp_status));

	exec_command("/usr/sbin/wpa_cli -i wlan0 status | grep ip_address", dhcp_status, sizeof(dhcp_status));

	if (dhcp_status[0]) {
		strcpy(status, "ok");
	} else {
		memset(dhcp_status, 0, sizeof(dhcp_status));
		exec_command("cat /tmp/dhcp_status", dhcp_status, sizeof(dhcp_status));
		if (!strncmp(dhcp_status, "running", strlen("running")))
			strcpy(status, "running");
		else if (!strncmp(dhcp_status, "fail", strlen("fail")))
			strcpy(status, "fail");
	}
	//ALOGD("dhcp_status:%s", dhcp_status);
	return 0;
}

bool checkWifiIsConnected()
{
    char ret_buff[1024] = {0};
	bool is_connected = false;
	bool is_vaild_ip_addr = false;
    bool isWifiConnected = false;
	int flag = 1;

	int connect_retry_count = 60;
	execute("ipaddr flush wlan0", ret_buff, 1024);

    /* 15s to check wifi whether connected */
    for (int i = 0; i < connect_retry_count; i++) {
        sleep(2);
		is_connected = false;
		is_vaild_ip_addr = false;

        execute("/usr/sbin/wpa_cli -iwlan0 status | grep wpa_state", ret_buff, 1024);
		//printf("wpa_cli -iwlan0 status | grep wpa_state: %s\n", ret_buff);
		if (strstr(ret_buff, "COMPLETED"))
			is_connected = true;

        execute("/usr/sbin/wpa_cli -iwlan0 status | grep ip_address", ret_buff, 1024);
		//printf("wpa_cli -iwlan0 status | grep ip_address: %s\n", ret_buff);
		if ((ret_buff[0]) && (strstr(ret_buff, "127.0.0.1") == NULL))
			is_vaild_ip_addr = true;

		if ((is_connected == true) && (is_vaild_ip_addr == false)) {
			system("echo running > /tmp/dhcp_status");
			if ((!(i%30)) || flag) {
				flag = 0;
				execute("/usr/bin/killall udhcpc", ret_buff, 1024);
				system("/bin/busybox udhcpc -i wlan0 -t 10 &");
				usleep(300000);
			}
		}

		if ((is_connected == true) && (is_vaild_ip_addr == true)) {
            isWifiConnected = true;
            // TODO play audio: wifi connected
            //printf("Congratulation: wifi connected.\n");
            break;
        }
		if (wifi_wrong_key)
			break;
		
        printf("Check wifi state with none state. try more %d/%d, \n",i+1, 60);
    }

	return isWifiConnected;
}


#define SECURITY_NONE	0
#define SECURITY_WEP	1
#define SECURITY_WPA	2

int connect_network2(char * iface, char * ssid, char * pw, int isAscii, int security, int isNew) {
	char buf[256] = {0};
	const int buf_len = sizeof(buf);
	char ret[8] = {0};
	int netId = -1;
	int i=0;
	int err = -1;
	
	getConnectedNetId(iface, buf);
	if (strcmp(ssid, buf) == 0) {
		ALOGD("%s is connected, skip connect_network2", ssid);
		return 0;
	}

	system("echo connecting > /tmp/wifi_status");
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "echo %s > /tmp/current_wifi", ssid);
	system(buf);

	if (isNew) {
		//add_network
		if (wpa_cli_cmd(iface, "add_network", ret, 16)) return -1;
		ALOGD("ap netId=%s", ret);
		netId = atoi(ret);

		//set_network ssid
		memset(buf, 0, buf_len);

		char ssid2[SSID_LEN_MAX*2+4];
		if(process_string_ssid(ssid, ssid2) < 0) return -1;
		sprintf(buf, "set_network %d ssid \\\"%s\\\"", netId, ssid2);
		ALOGD("buf: %s, ssid: %s\n", buf, ssid2);
		if (wpa_cli_cmd2(iface, buf)) goto err;
		
		if (strlen(pw) > 0) {
			if (SECURITY_WPA == security) {
				//set_network psk
				memset(buf, 0, buf_len);
				if (isAscii) {
					char pw_2[PW_LEN_MAX*2+1] = {0};
					if (process_string(pw, pw_2, PW_LEN_MAX*2+1) < 0) return -1;
					sprintf(buf, "set_network %d psk \\\"%s\\\"", netId, pw_2);
				} else {
					sprintf(buf, "set_network %d psk %s", netId, pw);
				}
				if (wpa_cli_cmd2(iface, buf)) goto err;
			} else if (SECURITY_WEP == security) {
				memset(buf, 0, buf_len);
				sprintf(buf, "set_network %d key_mgmt NONE", netId);
				if (wpa_cli_cmd2(iface, buf)) goto err;

				memset(buf, 0, buf_len);
				sprintf(buf, "set_network %d auth_alg \"OPEN SHARED\"", netId);
				if (wpa_cli_cmd2(iface, buf)) goto err;

				memset(buf, 0, buf_len);
				if (isAscii) {
					char pw_2[PW_LEN_MAX*2+1] = {0};
					if (process_string(pw, pw_2, PW_LEN_MAX*2+1) < 0) goto err;
					sprintf(buf, "set_network %d wep_key0 \\\"%s\\\"", netId, pw_2);
				} else {
					sprintf(buf, "set_network %d wep_key0 %s", netId, pw);
				}
				if (wpa_cli_cmd2(iface, buf)) goto err;
			}
		} else {
			memset(buf, 0, buf_len);
			sprintf(buf, "set_network %d key_mgmt NONE", netId);
			if (wpa_cli_cmd2(iface, buf)) goto err;
		}
	} else {
		netId = getNetIdBySsid(iface, ssid);
		if(netId < 0) {
			ALOGE("saved ssid %s not found", ssid);
			return -1;
		}
	}

	int connectedNetId = getConnectedNetId(iface, NULL);
	if (connectedNetId >= 0) {
		//disable_network
		memset(buf, 0, buf_len);
		sprintf(buf, "disable_network %d", connectedNetId);
		if (wpa_cli_cmd2(iface, buf)) goto err;

		char wifi_status[16];
		char dhcp_status[16];
		while(1) {
			if (!get_wifi_status(wifi_status, sizeof(wifi_status))) {
				if (!strncmp(wifi_status, "disconnected", strlen("disconnected")))
					ALOGD("previous connection is disconnected");
					break;
			}
			sleep(1);
		}
	}

	if (isNew == 2) { // hide ssid
		memset(buf, 0, buf_len);
		sprintf(buf, "set_network %d scan_ssid 1", netId);
		if (wpa_cli_cmd2(iface, buf)) goto err;
	}

	system("echo connecting > /tmp/wifi_status");
	//enable_network
	memset(buf, 0, buf_len);
	sprintf(buf, "select_network %d", netId);
	if (wpa_cli_cmd2(iface, buf)) goto err;
#if 0
	ALOGD("checking if ap %s connected...", ssid);
			
	while(1) {
		char ip[64];
		sleep(1);
		if(i++ > 20) break;
		//if(getConnectedNetId(iface) == netId) break;
		if (getIp(iface, ip)) break;
	}

	if(i > 20) goto err;
#endif
	//save_config
	if (wpa_cli_cmd2(iface, "save_config")) return -1;

	if (checkWifiIsConnected()) {
		system("echo idle > /tmp/dhcp_status");
		system("echo idle > /tmp/dhcp_status");
	} else {
		system("echo fail > /tmp/dhcp_status");
	}

	return 0;
err:
	
	if (isNew) {
		//remove_network
		memset(buf, 0, buf_len);
		sprintf(buf, "remove_network %d", netId);
		if (wpa_cli_cmd2(iface, buf)) return -1;

		//save_config
		if (wpa_cli_cmd2(iface, "save_config")) return -1;
	}

	return err;
}

int connect_network(char * iface, char * ssid, char * pw, int isAscii, char * flags, int isNew) {
	int security = 0;

	if (strstr(flags, "WEP") != NULL) {
		security = SECURITY_WEP;
	} else if (strstr(flags, "PSK") != NULL) {
		security = SECURITY_WPA;
	} else {
		security = SECURITY_NONE;
	}

	return connect_network2(iface, ssid, pw, isAscii, security, isNew);
}

int disconnect(char * iface, char * ssid) {
	int netId = -1;
	char buf[64] = {0};
	const int buf_len = 64;
	int i=0;
	char ssid2[64] = {0};

	if (process_string(ssid, ssid2, 64) < 0) return -1;

	netId = getNetIdBySsid(iface, ssid);

	if (netId < 0) return -1;

	//disable_network
	memset(buf, 0, buf_len);
	sprintf(buf, "disable_network %d", netId);
	wpa_cli_cmd2(iface, buf);

	sleep(2);

	return 0;
}

int forget(char * iface, char * ssid) {
	int netId = -1;
	char buf[64] = {0};
	const int buf_len = 64;
	int i=0;
	char ssid2[SSID_LEN_MAX*2+4] = {0};

	if (process_string(ssid, ssid2, SSID_LEN_MAX*2+4) < 0) return -1;

	netId = getNetIdBySsid(iface, ssid);

	if (netId < 0) return -1;

	//forget_network
	memset(buf, 0, buf_len);
	sprintf(buf, "remove_network %d", netId);
	if (wpa_cli_cmd2(iface, buf)) return -1;

	ALOGD("=========save_config");
	if (wpa_cli_cmd2(iface, "save_config")) return -1;


	sleep(2);

	return 0;
}

/*
return:
0: not connected
1: saved
2: connected
*/
int getNetworkState(char * iface, char * ssid) {
	struct list_head nwList;
	struct list_head * pList;
	int ret = 0;

	networkListInit(&nwList);
        getNetworkList(iface, &nwList);

	list_for_each(pList, &nwList) {
		struct list_network_s * nw = list_entry(pList, struct list_network_s, i_list);
		if(nw == NULL) continue;
		if(nw->ssid == NULL) continue;
		if (!strcmp(nw->ssid, ssid)) {
			ALOGD("network %s's flags: %s", nw->ssid, nw->flags);
			if(!strncmp(nw->flags, "[CURRENT]", strlen("[CURRENT]"))) {
				ret = 2;
				goto out;
			} else {
				ret = 1;
				goto out;
			}
		}
	}

out:
	clearNetworkList(&nwList);
	return ret;
}

 
