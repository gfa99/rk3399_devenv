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

#define LOG_TAG "ota_host.cgi"
#include "cutils/log.h"
#include "cutils/memory.h"
#include "cutils/misc.h"
#include "cutils/properties.h"
#include "../wifi/list_network.h"
#include "../wifi/lollipop_netTools.h"
#include "../utility/string_ext.h"
#include "../utility/list.h"
#include "../wifi/operate.h"
#include "../socket_ipc/lollipop_socket_ipc.h"

void parse(char *str, char * host) {
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
		if(str_startsWith(p, "host=")) {
			strcpy(host, p+strlen("host="));
		}
	}

	ALOGD("parse: host=%s", host);
}

int main(void) {
	char * lenstr;
	char input[512];
	long len;
	char host[256] = {0};
	char cmdBuf[256];

	printf("Content-Type:text/html\n\n");

	printf("<TITLE>set OTA host</TITLE>\n");
	lenstr = getenv("CONTENT_LENGTH");

	if(lenstr == NULL || sscanf(lenstr,"%ld",&len)!=1 ) {
		//printf("<P> wrong");
	} else {
		int i=0;
		fgets(input, len+1, stdin);
		ALOGD("recive post: %s\n", input);
		parse(input, host);

		lollipopConfInit();
		setLollipopConf("ota_host", host);
		saveLollipopConf();
		lollipopConfDeinit();

		system("/system/bin/reboot");
	}
	
	printf("<meta HTTP-EQUIV=refresh Content=\'0;url=settings.cgi\'>");
	return 0;
}
