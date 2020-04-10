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

int p2p_state(void) {
	struct list_head * pList;
	struct list_head groupList;

	printf("Content-type: text/html\n\n");
	printf("<HTML><BODY>\n");
	printf("<HEAD>\n");
	printf("<TITLE>Lollipop Home</TITLE>\n");
	printf("<link rel='stylesheet' type='text/css' href='css/main.css' />");
	printf("</HEAD>\n");

	printf("<p>");
	printf("<a href=\"wifi.cgi\"><img border=\"0\" src=\"/res/icon_wifi_01.png\" /></a>\n");
	printf("touch icon to show wifi direct group list");
	printf("</p>");
	printf("<br />\n");
	printf("<br />\n");

	printf("<p>");
	printf("<a href=\"key.cgi?control=switch\"><img border=\"0\" src=\"/res/icon_refresh_01.png\" /></a>\n");
	printf("touch icon to switch mode between Miracast and DLNA");
	printf("</p>");
	printf("<br />\n");
	printf("<br />\n");

	printf("<p>\n");
	printf("<a href=\"settings.cgi\"><img border=\"0\" src=\"/res/icon_set_01.png\" /></a>\n");
	printf("Settings\n");
	printf("</p>\n");
	printf("<br />\n");
	printf("<br />\n");


	printf("</BODY></HTML>\n");

	return 0;
}


int ap_state(void) {
	struct list_head apList;
	struct list_head * pList;
	
	printf("Content-type:text/html;charset=utf-8\n\n"); //response header.  
	printf("<HTML><BODY>\n");
	printf("<HEAD>\n");
	printf("<TITLE>Lollipop Home</TITLE>\n");
	printf("<link rel='stylesheet' type='text/css' href='/css/main.css' />");
	printf("</HEAD>\n");

	printf("<div class='content_icon'>\n");
	printf("<p><a href='wifi.cgi' class='icon_wifi'>WiFi AP</a></p>\n");
	printf("</div>\n\n");
	
	
	printf("<div class='content_icon'>\n");
	printf("<p><a href='key.cgi?control=switch' class='icon_switch'>Switch Mode</a></p>\n");
	printf("</div>\n");

	printf("<div class='content_icon'>\n");
	printf("<p><a href='media_control.cgi' class='icon_media'>Media Control</a></p>\n");
	printf("</div>\n");
	

	printf("<div class='content_icon'>\n");
	printf("<p><a href='settings.cgi' class='icon_settings'>Settings</a></p>\n");
	printf("</div>\n");
	
	printf("</BODY>\n");
	printf("</HTML>\n");

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
