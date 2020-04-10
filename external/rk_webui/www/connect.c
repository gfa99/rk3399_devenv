#include <stdio.h>
#include <stdlib.h>

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

//password=1234567890&ssid=link-sys&flags=[WEP][ESS]
void parse(char *str, char * pw, char * ssid, char * flags, char * coding) {
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
		if(str_startsWith(p, "password=")) {
//			strcpy(pw, p+strlen("password="));
			URLDecode(p+strlen("password="), pw, 128);
		} else if(str_startsWith(p, "ssid=")) {
//			strcpy(ssid, p+strlen("ssid="));
			URLDecode(p+strlen("ssid="), ssid, SSID_LEN_MAX+4);
		} else if (str_startsWith(p, "flags=")) {
			strcpy(flags, p+strlen("flags="));
		} else if (str_startsWith(p, "coding=")) {
			strcpy(coding, p+strlen("coding="));
		}
	}

	ALOGD("parse: ssid=%s, pw=%s", ssid, pw);
}

int main(void) {
	char * lenstr;
	char input[512];
	long len;
	char pw[PW_LEN_MAX+4] = {0};
	char ssid[SSID_LEN_MAX+4] = {0};
	char flags[128] = {0};
	char coding[8] = {0};
	int coding_flag = 1; //ascii

	printf("Content-Type:text/html\n\n");

	printf("<TITLE>connect to AP</TITLE>\n");
	lenstr = getenv("CONTENT_LENGTH");

	if(lenstr == NULL || sscanf(lenstr,"%ld",&len)!=1 ) {
		//printf("<P> wrong");
	} else {
		int i=0;
		fgets(input, len+1, stdin);
		ALOGD("recive post: %s\n", input);
		parse(input, pw, ssid, flags, coding);

		coding_flag = strncmp(coding, "hex", strlen("hex")) ? 1 : 0;
 
		if(!connect_network(WLAN_IFACE, ssid, pw, coding_flag, flags, 1)) {
			ALOGD("connected to %s \n", ssid);
		
		} else {
			ALOGD("connect to %s failed\n", ssid);
		}	
	}

	printf("<meta HTTP-EQUIV=refresh Content=\'0;url=wifi.cgi\'>");
	return 0;
}
