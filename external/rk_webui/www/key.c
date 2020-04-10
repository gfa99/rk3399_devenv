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

#include "../utility/common_web.h"

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
	char control[32];
	char buf[256];
	int keycode;
	ALOGD("got switch request from web...");

	printf("Content-type: text/html\n\n");
	printf("<HTML><BODY>\n");
	printf("<HEAD>\n");
	printf("<TITLE>Lollipop Mode Switch</TITLE>\n");
	printf("</HEAD>\n");

	char * env = getenv("QUERY_STRING");
	char * p = env;
	char *last;
	if(p == NULL) return -1;
	p = strtok_r(p, "=", &last);

	if(p == NULL) return -1;
	p = strtok_r(NULL, "=", &last);

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

	printf("</BODY></HTML>\n");

	return 0;
}
