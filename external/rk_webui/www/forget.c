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

#define DBG 1

int main(void) {
	char * env;
	long len;
	char ssid[SSID_LEN_MAX+4] = {0};
	char iface[16] = {0};
	char * p;
	char *last;

	printf("Content-Type:text/html\n\n");

	printf("<TITLE>forget AP</TITLE>\n");
	env = getenv("QUERY_STRING");

	p = strtok_r(env, "&", &last);
	while(p) {
		ALOGD("p=%s", p);
		if(!strncmp(p, "interface=", strlen("interface="))) {
			strcpy(iface, p+strlen("interface="));
			ALOGD("iface=%s", iface);
		} else if(!strncmp(p, "ssid=", strlen("ssid="))) {
			//strcpy(ssid, p+strlen("ssid="));
			URLDecode(p+strlen("ssid="), ssid, SSID_LEN_MAX+4);
			ALOGD("ssid=%s", ssid);
		}
		p = strtok_r(NULL, "&", &last);
	}

	forget(iface, ssid);
	
	printf("<meta HTTP-EQUIV=refresh Content=\'0;url=wifi.cgi\'>");
	return 0;
}
