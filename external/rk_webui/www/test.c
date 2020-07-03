#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>

int main(void)
{	
	printf("Content-type: text/html\n\n");

	printf("<HTML><HEAD>");
	printf("<TITLE>Test</TITLE>");

        system("mount -t vfat -o dmask=0000,fmask=0000 /dev/block/sda1 /mnt/external_sd");
        sleep(2);
        system("mediaplaytest -l -c /mnt/external_sd/test.mp4");

	printf("</HEAD></HTML>");
        return 0;
}
