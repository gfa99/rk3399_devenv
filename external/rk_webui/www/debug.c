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
#include <stdio.h>

int main(void) {
	int ret = -1;
	char * env;
	char input[512];
	int state = 0;
	int len = 0;

	printf("Content-type: text/html\n\n");
	printf("<HTML><BODY>\n");
	printf("<HEAD>\n");
	printf("<TITLE>Debug</TITLE>\n");
	printf("</HEAD>\n");

	//system("/system/bin/setprop sys.usb.config mass_storage,adb");

	printf("ADB enabled!\n");

	printf("</BODY></HTML>\n");

	return 0;
}
