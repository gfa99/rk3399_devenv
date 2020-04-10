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

#include "../utility/string_ext.h"
#include "../utility/list.h"
#include "../utility/common_web.h"
#include "../wifi/operate.h"
#include "../wifi/list_network.h"
#include "../wifi/lollipop_wifiScanAp.h"

//ssid=linksys&bssid=00:23:69:24:38:73&freq=2462&flags=[WPA-PSK-TKIP+CCMP][WPA2-PSK-TKIP+CCMP][WPS][ESS]
static void parse(char *str, char * ssid, char * bssid, char *freq, char * flags) {
	int i=0;
	char * p = str;
	char *last;

	while(p != NULL) {
		if(i++ == 0) {
			p = strtok_r(p, "&", &last);
		} else {
			p = strtok_r(NULL, "&", &last);
		}

		if (p == NULL) break;
		if(str_startsWith(p, "ssid=")) {
			//strcpy(ssid, p+strlen("ssid="));
			URLDecode(p+strlen("ssid="), ssid, SSID_LEN_MAX+4);
			ALOGD("after decode=%s", ssid);
		} else if(str_startsWith(p, "bssid=")) {
			strcpy(bssid, p+strlen("bssid="));
		} else if(str_startsWith(p, "freq=")) {
			strcpy(freq, p+strlen("freq="));
		} else if(str_startsWith(p, "flags=")) {
			strcpy(flags, p+strlen("flags="));
		} 
	}

	ALOGD("parse: ssid=%s", ssid);
	ALOGD("parse: bssid=%s", bssid);
	ALOGD("parse: freq=%s", freq);
	ALOGD("parse: flags=%s", flags);
}

int main(void) {
	int ret = -1;
	char * env;
	char input[512];
	char ssid[SSID_LEN_MAX+4] = {0};
	char bssid[32] = {0};
	char freq[16] = {0};
	char flags[256] = {0};
	int state = 0;
	int len = 0;

	printf("Content-type:text/html;charset=utf-8\n\n");
	printf("<HTML><BODY>\n");
	printf("<HEAD>\n");
	printf("<TITLE>AP Infomation</TITLE>\n");
	printf("<link rel='stylesheet' type='text/css' href='/css/main.css' />");
	printf("</HEAD>\n");

	env = getenv("QUERY_STRING");
	strcpy(input, env);
	if(strlen(input) == 0) {
		ALOGD("no post received");
	} else {
		ALOGD("recive post: %s\n", input);
		parse(input, ssid, bssid, freq, flags);
	}

	state = getNetworkState(WLAN_IFACE, ssid);
	
	printf("<div class='header'>\n");
  printf("	<a href='scan.cgi' class='backmenu'></a>\n");
  printf("	%s\n",ssid);
  printf("</div>\n");
  printf("<div class='content_box'>\n");
  printf("	<div class='content_list'>\n");
  printf("		<p> <strong>BSSID:</strong></p>\n");
  printf("		<p class='info'>%s</p>\n",bssid);
  printf("	</div>\n");
  printf("	<div class='content_list'>\n");
  printf("		<p><strong>Frequency:</strong></p>\n");
  printf("		<p class='info'>%sMhz</p>\n",freq);
  printf("	</div>\n");
  printf("	<div class='content_list no_solid'>\n");
  printf("		<p> <strong>Security:</strong></p>\n");
  printf("		<p class='info'>%s</p>\n",flags);
  printf("	</div>\n");
  printf("</div>\n");
  printf("<div class='content_box'>\n");
  printf("	<form action='connect.cgi' method='post' style='font-size:30px;'>\n");
  	char ssid2[SSID_LEN_MAX*4+4] = {0};
	URLEncode(ssid, ssid2, SSID_LEN_MAX*4+4);

	char ssid3[SSID_LEN_MAX*6+4] = {0};
	html_string_map(ssid, ssid3, SSID_LEN_MAX*6+4);
	if (state == 0) { //not saved
	//printf("ssid: %s, ssid2: %s, ssid3: %s\n", ssid, ssid2, ssid3);
  	printf("	<div class='content_input'>\n");
  	printf("		<input name='ssid' value='%s' type='hidden'>\n",ssid);
  	printf("		<input name='flags' value='%s' type='hidden'>\n",flags);
  	printf("	</div>\n");

		if (!isOpenWifi(flags)) {
			printf("	<div class='content_input'>\n");
			printf("Password:&nbsp;&nbsp;\n");
			printf("		<input name='password' type='text' class='password'>\n");
			printf("<br />\n");
			printf("<br />\n");
  		printf("		ASCII\n");
  		printf("		<input checked='checked' id='radio-1-1' name='coding' value='ascii' type='radio' class='regular-radio'><label for='radio-1-1'></label>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
  		printf("		HEX\n");
  		printf("		<input type='radio' id='radio-1-2' name='coding' value='hex' class='regular-radio' /><label for='radio-1-2'></label>\n");
  		
  		printf("	</div>\n");
  		
		}
		printf("\n");
    printf("\n");
    printf("\n");
		printf("	<center><input value='Connect' type='submit' class='menu_1'></center>\n");
		//printf("	</form>\n");
		//printf("</div>\n");
		
	} else if (state == 1) { //saved
		//printf("<font size=\"6\" face=\"arial\" color=\"red\">Saved</font>");
		//printf("<br />\n");
		//printf("<br />\n");
		printf("	<div class='content_input'>\n");
		printf("<br />\n");
		printf("<br />\n");

		printf("<center>\n");
		printf("<a class='menu_1' href=''>Saved</a>&nbsp;&nbsp;&nbsp;");
		//printf("	<a href='/cgi-bin/disconnect.cgi?ssid=mike%2Ddlink' class='menu_1'>Disconnect</a>&nbsp;&nbsp;&nbsp;");
		//printf("<a href='/cgi-bin/forget.cgi?ssid=mike%2Ddlink' class='menu_1'>Forget</a>");
		char cur_wifi[SSID_LEN_MAX];
		char wifi_status[16];
		memset(cur_wifi, 0, sizeof(cur_wifi));

		if (get_wifi_status(wifi_status, sizeof(wifi_status))) {
			ALOGE("get wifi status failed");
			return -1;
		}

		exec_command("cat /tmp/current_wifi", cur_wifi, sizeof(cur_wifi));
		ALOGD("current wifi: %s", cur_wifi);
		if ((!strncmp(cur_wifi, ssid, strlen(ssid))) && 
			(!strncmp(wifi_status, "connecting", strlen("connecting")))) {
			printf("<a class='menu_1' href=''>Connecting</a>&nbsp;&nbsp;&nbsp;");
			//printf("<font size=\"6\" face=\"arial\" color=\"red\">Connecting</font>");
		} else {
			printf("	<a href='/cgi-bin/connectSavedAp.cgi?ssid=%s' class='menu_1'>Connect</a>&nbsp;&nbsp;&nbsp;", ssid2);
			//printf("<a href=\"connectSavedAp.cgi?ssid=%s\"><font size=\"6\" face=\"arial\">Connect</font></a>\n", ssid2);
		}
		
		
		printf("<a href='forget.cgi?interface=%s&ssid=%s'class='menu_1'>Forget</a>\n", WLAN_IFACE, ssid2);
		printf("</center>\n");
		printf("	</div>\n");
	} else if (state == 2) { //connected
		char wifi_status[16];
		char dhcp_status[16];
		printf("	<div class='content_input'>\n");
		printf("<br />\n");
		printf("<br />\n");

		printf("<center>\n");

		if (!get_wifi_status(wifi_status, sizeof(wifi_status))) {
			if (!strncmp(wifi_status, "connecting", strlen("connecting"))) {
				printf("<a class='menu_1' href=''>Connecting</a>&nbsp;&nbsp;&nbsp;");
				/*
				printf("	<center><input value='Connect' class='menu_1' type='submit'>&nbsp;&nbsp;&nbsp;
				<a href='/cgi-bin/disconnect.cgi?ssid=mike%2Ddlink' class='menu_1'>Disconnect</a>&nbsp;&nbsp;&nbsp;
				<a href='/cgi-bin/forget.cgi?ssid=mike%2Ddlink' class='menu_1'>Forget</a></center>");
				*/

			} else if (!strncmp(wifi_status, "connected", strlen("connected"))) {
				if (!get_dhcp_status(dhcp_status, sizeof(dhcp_status))) {
					if (!strncmp(dhcp_status, "running", strlen("running"))) {
						printf("<a class='menu_1' href=''>Obtaining</a>&nbsp;&nbsp;&nbsp;");
						//printf("<font size=\"6\" face=\"arial\" color=\"red\">Obtaining IP</font>");
					} else if (!strncmp(dhcp_status, "ok", strlen("ok"))) {
						printf("<a class='menu_1' href=''>Connected</a>&nbsp;&nbsp;&nbsp;");
						//printf("<font size=\"6\" face=\"arial\" color=\"red\">Connected</font>");
					} else if (!strncmp(dhcp_status, "fail", strlen("fail"))) {
						printf("<a class='menu_1' href=''>NO IP</a>&nbsp;&nbsp;&nbsp;");
						//printf("<font size=\"6\" face=\"arial\" color=\"red\">NO IP</font>");
					} else {
						ALOGE("unknown dhcp satus--%s", dhcp_status);
					}
				} else {        
					printf("get dhcp status failed\n");
					ALOGE("get dhcp status failed");
				}
			} else {
				ALOGE("unknown wifi satus--%s", wifi_status);
			}
		} else {
			printf("get wifi status failed\n");
			ALOGE("get wifi status failed");
		}

		printf("<a href='/cgi-bin/disconnect.cgi?ssid=%s' class='menu_1'>Disconnect</a>&nbsp;&nbsp;&nbsp;", ssid2);
		printf("<a href='/cgi-bin/forget.cgi?ssid=%s' class='menu_1'>Forget</a>\n", ssid2);
		printf("</center>\n");
		printf("	</div>\n");		
	}  
  
  printf("	</form>\n");
  printf("</div>\n");

  printf("<div class='content_box'>\n");
  printf("	<div class='content_li'>\n");
  printf("	  <strong><font size='6' face='arial' color='#FF0000'>注意: 当连接目标WiFi和热点不在相同信道时会导致热点断开，请重新在setting界面连接热点，以获取配网状态。</font></strong>\n");
  printf("	</div>\n");
  printf("</div>\n");

	printf("</BODY></HTML>\n");

	return 0;
}
