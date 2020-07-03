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

	printf("Content-Type:text/html\n\n");

	printf("<TITLE>connect to saved AP</TITLE>\n");
	env = getenv("QUERY_STRING");

	ALOGD("recive post: %s\n", env);
//	strcpy(ssid, env+strlen("ssid="));
	URLDecode(env+strlen("ssid="), ssid, SSID_LEN_MAX+4);

	if(!connect_network(WLAN_IFACE, ssid, "", 1, "", 0)) {
		ALOGD("connected to %s \n", ssid);
	} else {
		ALOGE("connect to %s failed\n", ssid);
	}
	
	printf("<meta HTTP-EQUIV=refresh Content=\'0;url=wifi.cgi\'>");
	return 0;
}
