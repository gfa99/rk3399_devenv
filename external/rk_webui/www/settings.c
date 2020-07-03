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
			URLDecode(p+strlen("ssid="), ssid, 256);
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
        char overscan[8] = {0};
	char password[64] = {0};
	char language[64] = {0};
	char host[256] = {0};
	char freq[8] = {0};

        memset(overscan,  0, sizeof(overscan));
	memset(password, 0, sizeof(password));
	memset(language, 0, sizeof(language));
	memset(freq, 0, sizeof(freq));

	//lollipopConfInit();
	//getLollipopConf("fb_scale", overscan);
	//getLollipopConf("softap_password", password);
	//getLollipopConf("language", language);
	//getLollipopConf("ota_host", host);
	//getLollipopConf("softap_freq", freq);
	//lollipopConfDeinit();

	printf("Content-type:text/html;charset=utf-8\n\n");
	printf("<HTML><BODY>\n");
	printf("<HEAD>\n");
	printf("<TITLE>Settings</TITLE>\n");
	printf("<link rel='stylesheet' type='text/css' href='/css/main.css' />");
	printf("</HEAD>\n");
	printf("<div class='header'>\n");
  printf("	<a href='home.cgi' class='backmenu'></a>\n");
  printf("SETTING\n");
  printf("</div>\n");
  
  //overscan
  printf("<div class='content_box'>\n");
  printf("	<form action='/cgi-bin/overscan.cgi' method='post' style='font-size:30px;'>\n");
  printf("	<p class='set_txt'>Overscan:</p>\n");
  printf("	<p class='set_input'><input name='overscan' value='%s' type='text' class='password'></p>\n", overscan);
  printf("	<p class='set_input'><input value='Submit' type='submit' class='menu_1'></p>\n");
  printf("	</form>\n");
  printf("</div>\n");
  
  #ifdef SUPPORT_CHINESE_LANGUAGE
//language
	printf("<form action=\"/cgi-bin/language.cgi\" method=\"post\" style=\"font-size:30px;\">\n");
	printf("Language:\n");
	printf("(Dongle will be reboot)\n");
	printf("<br />\n");
	if (!strncmp(language, "English", strlen("English"))) {
		printf("<input type=\"radio\" checked=\"checked\" name=\"language\" value=\"English\"> English");
	} else {
		printf("<input type=\"radio\" name=\"language\" value=\"English\"> English");
	}

	printf("<br />\n");

	if (!strncmp(language, "Chinese", strlen("Chinese"))) {
		printf("<input type=\"radio\" checked=\"checked\" name=\"language\" value=\"Chinese\"> Chinese");
	} else {
		printf("<input type=\"radio\" name=\"language\" value=\"Chinese\"> Chinese");
	}

	printf("<br />\n");
	printf("<input type=\"submit\" value=\"Submit\" />\n");
	printf("<br />\n");
	printf("</form>\n");

	printf("<br />\n");
	printf("<br />\n");
#endif


//softap 2.4g or 5g
  printf("<div class='content_box'>\n");
  printf("	<div class='content_input'>\n");
  printf("		<form action='/cgi-bin/softap_freq.cgi' method='post' style='font-size:30px;'>\n");
  printf("		<p class='set_txt'>Softap Frequncy:(Dongle will be reboot)</p>\n");
  if (!strncmp(freq, "2.4G", strlen("2.4G"))) {
  	printf("		<p class='set_txt'>2.4G&nbsp;&nbsp;<input checked='checked' name='freq' value='2.4G' type='radio' id='radio-1-1' class='regular-radio'><label for='radio-1-1'></label>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
  	printf("5G&nbsp;&nbsp;<input name='freq' value='5G' type='radio' id='radio-1-2' class='regular-radio'><label for='radio-1-2'></label></p>\n");
  } else if (!strncmp(freq, "5G", strlen("5G"))){
    printf("		<p class='set_txt'>2.4G&nbsp;&nbsp;<input name='freq' value='2.4G' type='radio' id='radio-1-1' class='regular-radio'><label for='radio-1-1'></label>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
  	printf("5G&nbsp;&nbsp;<input checked='checked' name='freq' value='5G' type='radio' id='radio-1-2' class='regular-radio'><label for='radio-1-2'></label></p>\n");
  }
  else {
  	printf("		<p class='set_txt'>2.4G&nbsp;&nbsp;<input checked='checked' name='freq' value='2.4G' type='radio' id='radio-1-1' class='regular-radio'><label for='radio-1-1'></label>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
  	printf("5G&nbsp;&nbsp;<input name='freq' value='5G' type='radio' id='radio-1-2' class='regular-radio'><label for='radio-1-2'></label></p>\n");
  }
  
  printf("		<p class='set_input'><input value='Submit' type='submit' class='menu_1'></p>\n");
  printf("		</form>\n");
  printf("	</div>\n");
  printf("</div>\n");

//password
  printf("<div class='content_box'>\n");
  printf("	<form action='/cgi-bin/password.cgi' method='post' style='font-size:30px;'>\n");
  printf("	<p class='set_txt'>Password:(Dongle will be reboot)</p>\n");
  printf("	<p class='set_input'><input name='password' value='%s' type='text' class='password'></p>\n",password);
  printf("	<p class='set_input'><input value='Submit' type='submit' class='menu_1'></p>\n");
  printf("	</form>\n");
  printf("</div>\n");
  
  printf("<div class='content_box'>\n");
  printf("	<form action='/cgi-bin/ota_host.cgi' method='post' style='font-size:30px;'>\n");
  printf("	<p class='set_txt'>OTA host:(Dongle will be reboot)</p>\n");
  printf("	<p class='set_input'><input name='host' value='%s' type='text' class='password'></p>\n",host);
  printf("	<p class='set_input'><input value='Submit' type='submit' class='menu_1'></p>\n");
  printf("	</form>\n");
  printf("</div>\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
	/*

//overscan
	printf("<form action=\"/cgi-bin/overscan.cgi\" method=\"post\" style=\"font-size:30px;\">\n");
	printf("Overscan:");
	printf("<br />\n");
	printf("<input type=\"text\" name=\"overscan\" value=\"%s\">", overscan);
	printf("<br />\n");
	printf("<input type=\"submit\" value=\"Submit\" />\n");
	printf("<br />\n");
	printf("</form>\n");

	printf("<br />\n");
	printf("<br />\n");



//softap 2.4g or 5g
        printf("<form action=\"/cgi-bin/softap_freq.cgi\" method=\"post\" style=\"font-size:30px;\">\n");
        printf("Softap Frequncy:\n");
        printf("(Dongle will be reboot)\n");
        printf("<br />\n");
        if (!strncmp(freq, "2.4G", strlen("2.4G"))) {
                printf("<input type=\"radio\" checked=\"checked\" name=\"freq\" value=\"2.4G\"> 2.4G");
        } else {
                printf("<input type=\"radio\" name=\"freq\" value=\"2.4G\"> 2.4G");
        }

        printf("<br />\n");

        if (!strncmp(freq, "5G", strlen("5G"))) {
                printf("<input type=\"radio\" checked=\"checked\" name=\"freq\" value=\"5G\"> 5G");
        } else {
                printf("<input type=\"radio\" name=\"freq\" value=\"5G\"> 5G");
        }

        printf("<br />\n");
        printf("<input type=\"submit\" value=\"Submit\" />\n");
        printf("<br />\n");
        printf("</form>\n");

        printf("<br />\n");
        printf("<br />\n");

//password
	printf("<form action=\"/cgi-bin/password.cgi\" method=\"post\" style=\"font-size:30px;\">\n");
	printf("Password:");
	printf("(Dongle will be reboot)\n");
	printf("<br />\n");
	printf("<input type=\"text\" name=\"password\" value=\"%s\">", password);
	printf("<br />\n");
	printf("<input type=\"submit\" value=\"Submit\" />\n");
	printf("<br />\n");
	printf("</form>\n");

	printf("<br />\n");
	printf("<br />\n");

//host
	printf("<form action=\"/cgi-bin/ota_host.cgi\" method=\"post\" style=\"font-size:30px;\">\n");
	printf("OTA host:");
	printf("(Dongle will be reboot)\n");
	printf("<br />\n");
	printf("<input type=\"text\" name=\"host\" value=\"%s\">", host);
	printf("<br />\n");
	printf("<input type=\"submit\" value=\"Submit\" />\n");
	printf("<br />\n");
	printf("</form>\n");

	printf("<br />\n");
	printf("<br />\n");
*/

	printf("</BODY></HTML>\n");

	return 0;
}
