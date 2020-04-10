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

#define LOG_TAG "none_stop_service"
#include "cutils/log.h"
#include "cutils/memory.h"
#include "cutils/misc.h"
#include "cutils/properties.h"

int killService(char * process, int isKillMonitor)
{
	char tempKey[256] = {0};
	char tempValue[256] = {0};

	if (isKillMonitor) {

		memset(tempKey, 0, sizeof(tempKey));
		memset(tempValue, 0, sizeof(tempValue));

		sprintf(tempKey, "pid.monitor.%s", process);

		property_get(tempKey, tempValue, NULL);
		kill(atoi(tempValue), SIGKILL);
	}

	memset(tempKey, 0, sizeof(tempKey));
	memset(tempValue, 0, sizeof(tempValue));

	sprintf(tempKey, "pid.%s", process);

	property_get(tempKey, tempValue, NULL);
	kill(atoi(tempValue), SIGKILL);

	return 0;
}

static int savePidToProp(char * prefix, char * process, int processId) {
	char tempKey[128] = {0};
	char tempValue[16] = {0};

	//ALOGD("%s:%s:%d", prefix, process, processId);

	memset(tempKey, 0, sizeof(tempKey));
	memset(tempValue, 0, sizeof(tempValue));

	sprintf(tempKey, "%s.%s", prefix, strrchr(process, '/')+1);
	sprintf(tempValue, "%d", processId);

	property_set(tempKey, tempValue);
        return 0;
}


int service_loop(int argc, char ** argv)
{
	int i;
	char * child_argv[8] = {0};

	pid_t pid;
	int status;
	int ret;

	for (i = 1; i < argc; ++i) {  
		child_argv[i-1] = (char *)malloc(strlen(argv[i])+1);  
		strncpy(child_argv[i-1], argv[i], strlen(argv[i]));  
		child_argv[i-1][strlen(argv[i])] = 0;  
	}

	while(1){  
	        pid = fork();   
        	if (pid == -1) {  
				ALOGE(stderr, "fork() in while error.errno:%d error:%s", errno, strerror(errno));  
				continue;
        	}  
        	if (pid == 0) {  
			pid_t child_pid = getpid();

			savePidToProp("pid", child_argv[0], child_pid);

            		ret = execv(child_argv[0], (char **)child_argv);  
            		if (ret < 0) {  
						ALOGE(stderr, "execv ret:%d errno:%d error:%s", ret, errno, strerror(errno));
						exit(-1);
					}
        	}	  
   
        	if (pid > 0) {  
				pid = wait(&status);
        	}

                if (strstr(child_argv[0], "MediaDaemon") != NULL) {
			// when MediaDaemon died
			// need to restart rk_dlna_dmr, otherwise rk_dlna_dmr 
			// can't connect to MediaDaemon with binder
			ALOGD("MediaDaemon died, need to restart rk_dlna_dmr");
			killService("rk_dlna_dmr", 0);
                }
	} 
}


int main(int argc, char ** argv) {
	int i;
	pid_t pid_main;
	if (argv[0] == NULL) return -1;
	if (strlen(argv[0]) == 0) return -1;

	pid_main = fork();
	if (pid_main == -1) {
            	ALOGE(stderr, "fork() error.errno:%d error:%s", errno, strerror(errno));  
		return -1;
	} else if (pid_main == 0) {
		pid_t child_pid = getpid();
		savePidToProp("pid.monitor", argv[1], child_pid);
		service_loop(argc, argv);
	} else if (pid_main > 0) {
		return 0;
	}
        return 0;
}
