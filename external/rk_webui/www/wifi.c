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

#include "../wifi/list_network.h"
#include "../utility/string_ext.h"
#include "../utility/list.h"
#include "../wifi/operate.h"
#include "../utility/common_web.h"

int p2p_state(void) {
	struct list_head * pList;
	struct list_head groupList;

	printf("Content-type: text/html\n\n");
	printf("<HTML><BODY>\n");
	printf("<HEAD>\n");
	printf("<TITLE>Lollipop WiFi</TITLE>\n");
	printf("</HEAD>\n");

	networkListInit(&groupList);

	getNetworkList(P2P_IFACE, &groupList);

	printf("<font size=\"6\" face=\"arial\"><a href=\"home.cgi\">Back</a></font>\n");
	printf("<br />\n");
	printf("<br />\n");

	printf("<strong><font size=\"8\" face=\"arial\">Wifi P2P GROUP:</font></strong>\n");
	printf("<br />\n");
	printf("---------------------------------------------\n");
	printf("<br />\n");

	printf("<table cellpadding=\"10\" style=\"font-size:30px;\">\n");
	list_for_each(pList, &groupList) {
		struct list_network_s * grp = list_entry(pList, struct list_network_s, i_list);
		if(grp == NULL) continue;
		if(grp->ssid == NULL) continue;

		printf("<tr>\n");
		printf("<td style=\"background:#FFFFFF\">\n");
		printf("<strong><font size=\"6\" face=\"arial\">%s</font></strong>\n", grp->ssid);
		printf("</td>\n");

		printf("<td style=\"background:#FFFFFF\">\n");
		if(!strncmp(grp->flags, "[CURRENT]", strlen("[CURRENT]"))) {
			printf("<font color=\"red\">Connected</font>");
		}
		printf("</td>\n");

		printf("<td style=\"background:#FFFFFF\">\n");
		printf("<a href=\"forget.cgi?interface=%s&ssid=%s\">Forget</a>\n", P2P_IFACE, grp->ssid);
		printf("</td>\n");

		printf("</tr>\n");
	}

	printf("</table>\n");
	printf("</BODY></HTML>\n");

	clearNetworkList(&groupList);
	return 0;
}


int ap_state(void) {
	struct list_head apList;
	struct list_head * pList;

	printf("Content-type: text/html;charset=utf-8\n\n");
	printf("<HTML><BODY>\n");
	printf("<HEAD>\n");
	printf("<TITLE>Lollipop WiFi</TITLE>\n");
	printf("<link rel='stylesheet' type='text/css' href='/css/main.css' />");
	//printf("<meta http-equiv=\"refresh\" content=\"2\">\n");
	printf("</HEAD>\n");

	networkListInit(&apList);
	getNetworkList(WLAN_IFACE, &apList);


	printf("<div class='header'>\n");
  printf("	<a href='/cgi-bin/home.cgi' class='backmenu'></a>\n");
  printf("	Saved Access Points\n");
  printf("</div>\n");
  printf("<div class='content_box'>\n");
  printf("	<div class='content_li'>\n");
  printf("		<div class='content_list_wifi'>\n");
  printf("			<a href='/cgi-bin/scan.cgi'><strong><font size='8' face='arial'>Scan</font></strong></a>\n");
  //printf("			<a href='/cgi-bin/scan.cgi' class='wifi_scan' title='scan'></a>\n");
  printf("		</div>\n");
  printf("	</div>\n");
	list_for_each(pList, &apList) {
		struct list_network_s * ap = list_entry(pList, struct list_network_s, i_list);
		if(ap == NULL) continue;
		if(ap->ssid == NULL) continue;

		char ssid2[SSID_LEN_MAX*4+4] = {0};
		URLEncode(ap->ssid, ssid2, SSID_LEN_MAX*4+4);
  	printf("	<div class='content_li'>\n");
  	printf("		<div class='content_list_wifi'>\n");
  	printf("			%s\n", ap->ssid);
  	//printf("<strong><font size=\"8\" face=\"arial\">%s</font></strong>\n", ap->ssid);
  	printf("				<span style='float:right;'>");


		char wifi_status[16];
		char dhcp_status[16];
		if (get_wifi_status(wifi_status, sizeof(wifi_status))) {
			printf("get wifi status failed");
			ALOGE("get wifi status failed");
			break;
		}

		if (get_dhcp_status(dhcp_status, sizeof(dhcp_status))) {
			printf("get dhcp status failed\n");
			ALOGE("get dhcp status failed");
			break;
		}

		ALOGD("wifi_status:%s", wifi_status);
		ALOGD("dhcp_status:%s", dhcp_status);

		if(strncmp(ap->flags, "[CURRENT]", strlen("[CURRENT]"))) {
			char cur_wifi[SSID_LEN_MAX];
			memset(cur_wifi, 0, sizeof(cur_wifi));
			exec_command("cat /tmp/current_wifi", cur_wifi, sizeof(cur_wifi));
			ALOGD("current wifi: %s", cur_wifi);
			if ((!strncmp(cur_wifi, ap->ssid, strlen(ap->ssid))) && 
				(!strncmp(wifi_status, "connecting", strlen("connecting")))) {
				printf("<a class='menu_1' href=''>Connecting</a>&nbsp;&nbsp;");
			} else {
				printf("<a href='/cgi-bin/connectSavedAp.cgi?ssid=%s' class='menu_1'>Connect</a>&nbsp;&nbsp;", ssid2);
			}
		} else {
			if (!strncmp(wifi_status, "connecting", strlen("connecting"))) {
				printf("<a class='menu_1' href=''>Connecting</a>&nbsp;&nbsp;");
				//printf("<font color=\"red\">Connecting</font>");
			} else if (!strncmp(wifi_status, "connected", strlen("connected"))) {
				if (!strncmp(dhcp_status, "running", strlen("running"))) {
					printf("<a class='menu_1' href=''>Obtaining</a>&nbsp;&nbsp;");
					//printf("<font color=\"red\">Obtainng IP</font>");
				} else if (!strncmp(dhcp_status, "ok", strlen("ok"))) {
					printf("<a class='menu_1' href=''>Connected</a>&nbsp;&nbsp;");
				} else if (!strncmp(dhcp_status, "fail", strlen("fail"))) {
					printf("<a class='menu_1' href=''>NO IP</a>");
					//printf("<font color=\"red\">No IP</font>");
				} else {
					ALOGE("unknown dhcp satus--%s", dhcp_status);
				}
			} else {
				ALOGE("unknown wifi satus--%s", wifi_status);
			}
		}

			printf("<a class='menu_1' href='/cgi-bin/forget.cgi?interface=wlan0&amp;ssid=%s'>Forget</a>&nbsp;&nbsp;", ssid2);

		if(!strncmp(ap->flags, "[CURRENT]", strlen("[CURRENT]"))) {
			printf("<a class='menu_1' href='/cgi-bin/disconnect.cgi?ssid=%s'>Disconnect</a>", ssid2);
		}
		
  	printf("</span>\n");
  	
  	printf("		</div>\n");
  	printf("	</div>\n");
	}


  printf("<div class='content_box'>\n");
  printf("	<div class='content_li'>\n");
  printf("		<div class='content_list_wifi'>\n");
  printf("			<a href='/cgi-bin/add_network.cgi'><strong><font size='8' face='arial'>Add Network</font></strong></a>\n");

  printf("		</div>\n");
  printf("	</div>\n");
	printf("</div>\n");

	//printf("<div class='content_box'>\n");
	//printf("  <div class='content_li'>\n");
	//printf("	<strong><font size='6' face='arial' color='#FF0000'>注意: 当连接目标WiFi和热点不在相同信道时会导致热点断开，请重新在setting界面连接热点，以获取配网状态。</font></strong>\n");
	//printf("  </div>\n");
	//printf("</div>\n");

	printf("</BODY></HTML>\n");

	clearNetworkList(&apList);
	return 0;
}

int main(void) {
	char mode[32] = {0};

	memset(mode,  0, sizeof(mode));
	//lollipopConfInit();
	//getLollipopConf("function_mode", mode);
	//lollipopConfDeinit();
	return ap_state();

	//ALOGD("mode %s", mode);
	//if (!strcmp(mode, "WFD")) {
	//	return p2p_state();
	//} else if (!strcmp(mode, "DLNA")) {
	//	return ap_state();
	//} else {
	//	return -1;
	//}
}
