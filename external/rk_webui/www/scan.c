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
#include "../wifi/operate.h"
#include "../utility/common_web.h"
#include "../wifi/list_network.h"
#include "../wifi/lollipop_wifiScanAp.h"

int main(void) {
	struct list_head * pList;
	struct list_head scanApList;
	int ret = -1;

	printf("Content-type:text/html;charset=utf-8\n\n");
	printf("<HTML><BODY>\n");
	printf("<HEAD>\n");
	printf("<TITLE>wifi AP list</TITLE>\n");
	printf("<link rel='stylesheet' type='text/css' href='/css/main.css' />");
	printf("</HEAD>\n");

	scanApListInit(&scanApList);

	scan(WLAN_IFACE, &scanApList);
	
	printf("<div class='header'>\n");
  printf("	<a href='wifi.cgi' class='backmenu'></a>\n");
  printf("	Scan Results\n");
  printf("</div>\n");
  
  printf("<div class='content_box'>\n");
  int j = 0;
	int k = 0;
	list_for_each(pList, &scanApList) {
                struct scan_result_s *ap = list_entry(pList, struct scan_result_s, i_list);
		if(ap == NULL) continue;
		if(ap->ssid == NULL) continue;
		if(strlen(ap->ssid) == 0) continue;

		int level = calculateSignalLevel(ap->level, RSSI_LEVELS);

		char ssid2[SSID_LEN_MAX*4+4] = {0};
		URLEncode(ap->ssid, ssid2, SSID_LEN_MAX*4+4);
		ALOGD("parse: ssid=%s ,level=%d", ssid2,level);

		//if (isOpenWifi(ap->flags)) {		}
		printf("	<div class='content_li'>\n");

 		printf("		<div class='content_list_wifi'>\n");
  	printf("			<a class='wifi_title' href='dialog.cgi?ssid=%s&amp;bssid=%s&amp;freq=%d&amp;flags=%s'>%s</a>\n",ssid2, ap->bssid, ap->freq, ap->flags, ap->ssid);
  	
  	switch (level){
  		case 0:
  			printf("			<span class='wifi_sign'><img src='/res/wifi_01.png'></span>\n");
  			break;
  		case 1:
  			printf("			<span class='wifi_sign'><img src='/res/wifi_01.png'></span>\n");
  			break;
  		case 2:
  			printf("			<span class='wifi_sign'><img src='/res/wifi_01.png'></span>\n");
  			break;
  		case 3:
  			printf("			<span class='wifi_sign'><img src='/res/wifi_02.png'></span>\n");
  			break;
  		case 4:
  			printf("			<span class='wifi_sign'><img src='/res/wifi_03.png'></span>\n");
  			break;
  		case 5:
  			printf("			<span class='wifi_sign'><img src='/res/wifi_04.png'></span>\n");
  			break;
  		default:
  			printf("			<span class='wifi_sign'><img src='/res/wifi_04.png'></span>\n");
  			break;
  			
  	}
  	printf("		</div>\n");
		printf("	</div>\n");

  	
	}

  printf("</div>\n");
 	printf("</BODY></HTML>\n");

	clearScanApList(&scanApList);
	return 0;
}
