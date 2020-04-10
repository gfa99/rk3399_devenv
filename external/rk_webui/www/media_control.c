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

#define SWITCH_MODE	115
#define LONG_PRESS	119

#define MEDIA_PLAY_PAUSE	402
#define MEDIA_PLAY		64	
#define MEDIA_PAUSE		65
#define MEDIA_STOP		66
#define MEDIA_REW		67
#define MEDIA_FFW		68
#define MEDIA_PRE		87
#define MEDIA_NEXT		88
#define MEDIA_VOL_UP		116	
#define MEDIA_VOL_DOWN		114
#define MEDIA_VOL_MUTE		113


int main(void) {
	printf("Content-type: text/html;charset=utf-8\n\n");
	printf("<HTML><BODY>\n");
	printf("<HEAD>\n");
	printf("<TITLE>Lollipop Media Control</TITLE>\n");
	printf("<link rel='stylesheet' type='text/css' href='/css/main.css' />");
	printf("</HEAD>\n");
  
  printf("<div class='header'>\n");
  printf("<a href='home.cgi' class='backmenu'></a>\n");
  printf("</div>\n");
  printf("<div class='content_box'>\n");
  printf("	<div class='control_menu_1'>\n");
  printf("		<table>\n");//cellpadding='0' cellspacing='0' border='0' width='100%'
  printf("			<tr>\n");
  printf("				<td align='center' valign='bottom'>\n");
  printf("					<a href='/cgi-bin/media_control.cgi?control=vol_down' class='control_vol_down'></a>\n");
  printf("				</td>\n");
  printf("				<td align='center'>\n");
  printf("					<a href='/cgi-bin/media_control.cgi?control=play' class='control_play'></a>\n");
  printf("					<!--ke test 22224444444444444444444-->\n");
  printf("					<a href='/cgi-bin/media_control.cgi?control=pause' class='control_pause'></a>\n");
  printf("				</td>\n");
  printf("				<td align='center' valign='bottom'>\n");
  printf("					<a href='/cgi-bin/media_control.cgi?control=vol_up' class='control_vol_up'></a>\n");
  printf("				</td>\n");
  printf("			</tr>\n");
  printf("		</table>\n");
  printf("	</div>\n");
  printf("	<div class='control_menu_2'>\n");
  printf("		<table>\n");
  printf("			<tr>\n");
  printf("				<td align='center'><a href='/cgi-bin/media_control.cgi?control=pre' class='control_pre'></a></td>\n");
  printf("				<td align='center'><a href='/cgi-bin/media_control.cgi?control=rew' class='control_rew'></a></td>\n");
  printf("				<td align='center'><a href='/cgi-bin/media_control.cgi?control=stop' class='control_stop'></a></td>\n");
  printf("				<td align='center'><a href='/cgi-bin/media_control.cgi?control=ffw' class='control_ffw'></a></td>\n");
  printf("				<td align='center'><a href='/cgi-bin/media_control.cgi?control=next' class='control_next'></a></td>\n");
  printf("			</tr>\n");
  printf("		</table>\n");
  printf("	</div>\n");
  printf("</div>\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
  printf("\n");
    /*
	printf("<font size=\"6\" face=\"arial\"><a href=\"home.cgi\">Back</a></font>\n");
        printf("<br />\n");
        printf("<br />\n");
	printf("<table>\n");
		printf("<tr>\n");
		printf("<td>\n");
		printf("<a href=\"media_control.cgi?control=play\">\n");
		printf("<img border=\"0\" src=\"/res/play.png\" />\n");
		printf("</a>\n");
		printf("</td>\n");

		printf("<td>\n");
		printf("<a href=\"media_control.cgi?control=pause\">\n");
		printf("<img border=\"0\" src=\"/res/pause.png\" />\n");
		printf("</a>\n");
		printf("</td>\n");

		printf("<td>\n");
		printf("<a href=\"media_control.cgi?control=stop\">\n");
		printf("<img border=\"0\" src=\"/res/stop.png\" />\n");
		printf("</a>\n");
		printf("</td>\n");

		printf("</tr>\n");
//===============
		printf("<tr>\n");

		printf("<td>\n");
		printf("<a href=\"media_control.cgi?control=rew\">\n");
		printf("<img border=\"0\" src=\"/res/rew.png\" />\n");
		printf("</a>\n");
		printf("</td>\n");

		printf("<td>\n");
		printf("<a href=\"media_control.cgi?control=ffw\">\n");
		printf("<img border=\"0\" src=\"/res/ffw.png\" />\n");
		printf("</a>\n");
		printf("</td>\n");

		printf("<td>\n");
		printf("<a href=\"media_control.cgi?control=vol_up\">\n");
		printf("<img border=\"0\" src=\"/res/vol_up.png\" />\n");
		printf("</a>\n");
		printf("</td>\n");


		printf("</tr>\n");

//===============
		printf("<tr>\n");

		printf("<td>\n");
		printf("<a href=\"media_control.cgi?control=pre\">\n");
		printf("<img border=\"0\" src=\"/res/pre.png\" />\n");
		printf("</a>\n");
		printf("</td>\n");

		printf("<td>\n");
		printf("<a href=\"media_control.cgi?control=next\">\n");
		printf("<img border=\"0\" src=\"/res/next.png\" />\n");
		printf("</a>\n");
		printf("</td>\n");

		printf("<td>\n");
		printf("<a href=\"media_control.cgi?control=vol_down\">\n");
		printf("<img border=\"0\" src=\"/res/vol_down.png\" />\n");
		printf("</a>\n");
		printf("</td>\n");

		printf("</tr>\n");

	printf("</table>\n");
*/


        char * env = getenv("QUERY_STRING");
        char * p = env;
	char *last;
        if(p != NULL) {
        	p = strtok_r(p, "=", &last);
	        if(p != NULL) {
	        	p = strtok_r(NULL, "=", &last);
			if (p != NULL) {
				char buf[256];
				int keycode;

				ALOGD("control: %s", p);

				memset(buf, 0, sizeof(buf));

				if (!strcmp(p, "switch")) {
					keycode = SWITCH_MODE;
				} else if (!strcmp(p, "play")) {
					keycode = MEDIA_PLAY;
				} else if (!strcmp(p, "pause")) {
					keycode = MEDIA_PAUSE;
				} else if (!strcmp(p, "stop")) {
					keycode = MEDIA_STOP;
				} else if (!strcmp(p, "ffw")) {
					keycode = MEDIA_FFW;
				} else if (!strcmp(p, "rew")) {
					keycode = MEDIA_REW;
				} else if (!strcmp(p, "next")) {
					keycode = MEDIA_NEXT;
				} else if (!strcmp(p, "pre")) {
					keycode = MEDIA_PRE;
				} else if (!strcmp(p, "vol_up")) {
					keycode = MEDIA_VOL_UP;
				} else if (!strcmp(p, "vol_down")) {
					keycode = MEDIA_VOL_DOWN;
				}

				sprintf(buf, "/system/bin/tvcontrol -k %d", keycode);
				system(buf);
			}
		}
	}
	printf("</BODY></HTML>\n");

	return 0;
}


