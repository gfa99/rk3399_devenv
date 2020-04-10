#ifndef _LOLLIPOP_SOFTAP_H
#define _LOLLIPOP_SOFTAP_H

#include <linux/in.h>
#include <net/if.h>

#define SOFTAP_MAX_BUFFER_SIZE	4096
#define AP_BSS_START_DELAY	200000
#define AP_BSS_STOP_DELAY	500000
#define AP_SET_CFG_DELAY	500000
#define AP_DRIVER_START_DELAY	800000

extern void init_softap(void);
extern void deinit_softap(void);
extern int startSoftap(void);
extern int stopSoftap(void);
extern int isSoftapStarted();
extern int setSoftap(int argc, char *argv[]);
extern void generatePsk(char *ssid, char *passphrase, char *psk);
extern int fwReloadSoftap(int argc, char *argv[]);
extern int clientsSoftap(char **retbuf);
#endif //_LOLLIPOP_SOFTAP_H
