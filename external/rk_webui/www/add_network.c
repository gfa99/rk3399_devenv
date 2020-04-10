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

//#define LOG_TAG "add_network.cgi"
//#include "cutils/log.h"
//#include "cutils/memory.h"
//#include "cutils/misc.h"
//#include "cutils/properties.h"
//#include "../wifi/list_network.h"
//#include "../wifi/lollipop_netTools.h"
#include "../utility/string_ext.h"
#include "../utility/list.h"
//#include "../wifi/operate.h"

int main(void) {
	int ret = -1;
	int security = 2; // 0: None; 1: WEP; 2: WPA/WPA2 PSK
	char chk1[32], chk2[32], chk3[32];
	
	memset(chk1, 0, 32);
	memset(chk2, 0, 32);
	memset(chk3, 0, 32);

	printf("Content-type: text/html\n\n");
	printf("<HTML><BODY>\n");
	printf("<HEAD>\n");
	printf("<TITLE>Add Network</TITLE>\n");
	printf("<link rel='stylesheet' type='text/css' href='/css/main.css' />");
	printf("</HEAD>\n");
	
	printf("<div class='header'>\n");
  printf("	<a href='/cgi-bin/wifi.cgi' class='backmenu'></a>\n");
  printf("</div>\n");
  
  printf("<div class='content_box'>\n");
  printf("	<div class='content_input'>\n");
  printf("		<form action='/cgi-bin/add_network_connect.cgi' method='post'>\n");
  printf("		<p class='set_txt'>Network SSID:</p>\n");
  printf("		<p class='set_input'><input name='ssid' type='text' class='password'></p>\n");
  printf("		<p class='set_txt'>Security:</p>\n");
  switch (security) {
    	case 0:
            strcpy(chk1, "checked=\"checked\"");
            break;
    	case 1:
            strcpy(chk2, "checked=\"checked\"");
            break;
    	case 2:
            strcpy(chk3, "checked=\"checked\"");
            break;
    }
  printf("		<p class='set_input'>\n");
  printf("			None&nbsp;&nbsp;<input checked='checked' name='security' value='None' type='radio' id='radio-1-1' class='regular-radio'><label for='radio-1-1'></label>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
  printf("			WEP&nbsp;&nbsp;<input name='security' value='WEP' type='radio' id='radio-1-2' class='regular-radio'><label for='radio-1-2'></label>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
  printf("			WPA/WPA2&nbsp;&nbsp;<input checked='checked' name='security' value='WPA/WPA2' type='radio' id='radio-1-3' class='regular-radio'><label for='radio-1-3'></label>\n");
	printf("		</p>\n");
  printf("		<p class='set_txt'>Password:</p>\n");
  printf("		<p class='set_input'><input name='password' type='text' class='password'></p>\n");
  printf("		<p class='set_input'><input value='Submit' type='submit' class='menu_1'></p>\n");
  printf("		</form>\n");
  printf("	</div>\n");
  printf("</div>\n");  
	printf("</BODY></HTML>\n");

	return 0;
}
