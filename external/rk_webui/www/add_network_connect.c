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

#include "../wifi/list_network.h"
#include "../utility/string_ext.h"
#include "../utility/list.h"
#include "../wifi/operate.h"
#include "../utility/common_web.h"

void parse(char *str, char * ssid, char * security, char * password, char * coding) {
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
			URLDecode(p+strlen("password="), password, 128);
		} else if(str_startsWith(p, "ssid=")) {
			URLDecode(p+strlen("ssid="), ssid, SSID_LEN_MAX+4);
		} else if (str_startsWith(p, "security=")) {
			strcpy(security, p+strlen("security="));
		} else if (str_startsWith(p, "coding=")) {
			strcpy(coding, p+strlen("coding="));
		}
	}

	ALOGD("parse: ssid=%s, security=%s, password = %s, coding = %s", ssid, security, password, coding);
}

int main(void) {
	char * lenstr;
	char input[512];
	long len;
	char password[PW_LEN_MAX+4] = {0};
	char ssid[SSID_LEN_MAX+4] = {0};
	char security[128] = {0};
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
		parse(input, ssid, security, password, coding);
 
 		if (strstr(security, "WPA") != NULL) {
 			strcpy(security, "PSK");
 		}
 		//coding_flag = strncmp(coding, "hex", strlen("hex")) ? 1 : 0;
		if(!connect_network(WLAN_IFACE, ssid, password, coding_flag, security, 2)) {
			ALOGD("connected to %s \n", ssid);
		
		} else {
			ALOGD("connect to %s failed\n", ssid);
		}	
	}

	printf("<meta HTTP-EQUIV=refresh Content=\'0;url=wifi.cgi\'>");
	return 0;
}
