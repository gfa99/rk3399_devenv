#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#define LOG_TAG "playUrl.cgi"
#include "cutils/log.h"
#include "cutils/memory.h"
#include "cutils/misc.h"
#include "cutils/properties.h"
#include "../utility/string_ext.h"


//url=http:\\www.xxx.com\xxx.mp4
void parse(char *str, char * url, int url_len, int * isLoop) {
	int i=0;
	char * p = str;
	char *last;

	*isLoop = 0;

	while(p != NULL) {
		if(i++ == 0) {
			p = strtok_r(p, "&", &last);
		} else {
			p = strtok_r(NULL, "&", &last);
		}

		if (p == NULL) break;
		if(str_startsWith(p, "url=")) {
			//strcpy(url, p+strlen("url="));
			URLDecode(p+strlen("url="), url, url_len);
		} else if (str_startsWith(p, "loop=")) {
			ALOGD("%s", p);
			*isLoop = 1;

		}
	}

	ALOGD("parse: url=%s", url);
}



int main(void)
{	
	char * buf;
	char * url;
	char * lenstr;
	int len;
	int isLoop;

	printf("Content-Type:text/html\n\n");

	printf("<TITLE>Play URL</TITLE>\n");
        lenstr = getenv("CONTENT_LENGTH");
                        
        if(lenstr == NULL || sscanf(lenstr,"%ld",&len)!=1) {
                printf("<P> wrong");
        } else {        
                int i=0;
		buf = (char *)malloc(len+32);
		url = (char *)malloc(len);
		fgets(buf, len+1, stdin);
		ALOGD("recive post: %s\n", buf);

		parse(buf, url, len, &isLoop);

		memset(buf, 0, len+32);
		
		FILE * fp;
		const char * video_url = "/data/video_url";
		fp = fopen(video_url, "w+");
		if (fp != NULL) {
			fwrite(url, 1, strlen(url), fp);
			fclose(fp);
			if (isLoop) {
        			sprintf(buf, "/system/bin/mediaplayertest -f %s -l", video_url);
			} else {
        			sprintf(buf, "/system/bin/mediaplayertest -f %s", video_url);
			}
			ALOGD("%s", buf);
			system(buf);
		} else {
			ALOGD("open file %s failed", video_url);
		}
	}

	printf("</HEAD></HTML>");
	return 0;
}
