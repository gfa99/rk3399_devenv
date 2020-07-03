#ifndef __RK_SYSTEM_H__
#define __RK_SYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 *Version 1.3.1 Release 2019/11/16
  bt source callback add device address and device name
 *Version 1.3.0 Release 2019/10/30
  1.bluez: add rk_bt_deinit api
  2.bluez: fix get lcoal device name and bt mac address api
  3.blueZ: support pbap profile
  4.bluez: support hfp 16K sample rate
  5.bluez: support sink underrun msg report
  6.bsa: support set bt mac address in rk_bt_init
  7.bsa: add rk_bt_sink_set_alsa_device api
  8.bsa: add rk_bt_ble_set_visibility, rk_ble_set_local_privacy api
  9.add bt state report
  10.add bt pair state report
  11.add rk_bt_start_discovery, rk_bt_cancel_discovery, rk_bt_is_discovering, rk_bt_display_devices api
  12.add rk_bt_pair_by_addr, rk_bt_unpair_by_addr api
  13.add rk_bt_get_paired_devices, rk_bt_free_paired_devices, rk_bt_display_paired_devices api
  14.add rk_bt_set_device_name api
  15. add song track info report
  16. add song playing progress change report
  17. add avdtp(a2dp sink) state report
  18. add rk_bt_sink_connect_by_addr, rk_bt_sink_disconnect_by_addr api
  19. add rk_bt_sink_get_play_status, rk_bt_sink_get_poschange api
  20. support log to syslog
 *Version 1.2.4 Release 2019/06/24
  1.add alsa control demo
  2.add rk_bt_hfp_disconnect api
  3.fix PICKUP and HANGUP events cannot be received when answering or rejecting calls on the phone
  4.bsa: add hfp enable cvsd(8k samplerate) api
  5.bsa: fix the cypress bsa pairing popup prompt
  6.bsa: update broadcom bsa version to rockchip_20190617
  7.bsa: fix can't identify individual COD while bluetooth scanning
  8.bsa: fix battery power report bug
 *Version 1.2.3 Release 2019/06/05
  1.bluez:add a2dp sink volume api
  2.bluez:fix rk_bt_sink_set_visibility api
  3.bluez:fix rk_bt_enable_reconnect api
  4.bluez:cancel spp's dependence on a2dp sink
  5.bluez:fix automatic reconnection coredump
  6.bsa: fix a2dp source can't auto-connect sink devices
  7.bsa：fix rk_bt_hfp_hangup api
 *Version 1.2.2 Release 2019/05/27
  1.add bt source avrcp function
  2.add hfp hf api
  3.add set bt device class api
  4.add auto reconnect for sink & hfp
  5.add softap network config
  6.add airkiss network config
 *Version 1.2.1 Release 2019/04/29
  1.deviceio_test blewifi: register ble data recv callback(BSA)
  2.fix bug with program stuck when starting bluetooth related service(BLUEZ)
  3.fixed a bug that failed to get a bluetooth device role(BLUEZ)
 *Version 1.2.0 Release 2019/04/15
  1. bluethood api: add support for bsa.
  2. add support for deviceio_test.
 *Version 1.1.0 Release 2019/03/27
  1.rewrite the bt api include ble/a2dp/spp
 *Version 1.0.4 Release 2019/01/29
  1.ble_wifi
    1. add config timeout to discon ble
    2. fix gatt_open flag
    3. add reconfigure when last config fail
 *Version 1.0.3 Release 2019/01/24
  1.ble_wifi
    1. fix netstatus print error when NETLINK_NETWORK_CONFIG_WRONG_KEY_FAILED
    2. notify NETLINK_NETWORK_SUCCEEDED when net recovery from config failed
 *Version 1.0.2 Release 2019/01/24
  1.ble_wifi
    1. support wep key
    2. optimize wpa_supplicant.conf
    3. fix many times reports sink connected
    4. add ble_config wifi timeout value
    5. fix chinese code for hisense
  2.Rk_key: add fix register long press callback with different keyevent

 *Version 1.0.1 Release 2019/01/07
  1.ble_wifi:	fix Chinese coding problem
		fix ble report event and add wifi priority
		add wrong key event callback
		add initBTForHis interface
  2.volume:	setVolume support zero
  3.propery:	implement RK_property
  4.player:	separate mediaplayer and deviceio
		add playlist function
 *Version 1.0.0 Release 2018/12/22
 */

#define DEVICEIO_VERSION "V1.3.1"

int RK_read_chip_id(char *buffer, const int size);
int RK_read_version(char *buffer, const int size);
int RK_system_factory_reset(const int reboot = 1);


#ifdef __cplusplus
}
#endif

#endif
