#include <stdio.h>
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
#include <sys/time.h>
#include <signal.h>
#include "../utility/common_web.h"

#define DBG 0

int exec_command(char * in, char * out, int outLen) {
        FILE * fstream;
        int readLen = 0;

        if(in == NULL) return -1;
        if(out == NULL) return -1;

        if(DBG) ALOGD("command: %s", in);
        if(NULL==(fstream=popen(in,"r"))) {
                ALOGE("execute command \"%s\" failed: %s", in, strerror(errno));
                return -1;
        }

        readLen = fread(out, 1, outLen, fstream);

        if(readLen <= 0) return -1;
        if(DBG) ALOGD("command return len=%d", strlen(out));
        if(DBG) ALOGD("command %s return: \n%s", in, out);

        pclose(fstream);
        return 0;
}


