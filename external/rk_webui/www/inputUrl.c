#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
	printf("Content-type: text/html\n\n");
	printf("<HTML><BODY>\n");
	printf("<HEAD>\n");
	printf("<TITLE>AP Infomation</TITLE>\n");
	printf("</HEAD>\n");

	printf("<form action=\"/cgi-bin/playUrl.cgi\" method=\"post\" style=\"font-size:30px;\">\n");

	printf("URL:");
	printf("<br />\n");
	printf("<textarea name=\"url\" rows=\"10\" cols=\"30\"></textarea>");
//	printf("<input type=\"text\" name=\"url\">");
	printf("<br />\n");
	printf("<input type=\"checkbox\" name=\"loop\" />Repeat");
	printf("<br />\n");
	printf("<br />\n");
	printf("<input type=\"submit\" value=\"Play\" />\n");
	printf("<br />\n");
	printf("</form>\n");	

	printf("</BODY></HTML>\n");

	return 0;
}
