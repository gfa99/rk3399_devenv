#ifndef __BT_BASE_H__
#define __BT_BASE_H__

#include <DeviceIo/bt_manager_1s2.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

typedef struct {
#define UUID_16     2
#define UUID_32     4
#define UUID_128    16

	uint16_t len; //byte
	const char *uuid;
} Ble_Uuid_Type_t;

enum {
	BLE_ADVDATA_TYPE_USER = 0,
	BLE_ADVDATA_TYPE_SYSTEM
};

/*BT state*/
typedef enum {
	RK_BT_STATE_OFF,
	RK_BT_STATE_ON,
	RK_BT_STATE_TURNING_ON,
	RK_BT_STATE_TURNING_OFF,
} RK_BT_STATE;

typedef enum {
	RK_BT_BOND_STATE_NONE,
	RK_BT_BOND_STATE_BONDING,
	RK_BT_BOND_STATE_BONDED,
} RK_BT_BOND_STATE;

/*BT discovery state*/
typedef enum {
	RK_BT_DISC_STARTED,
	RK_BT_DISC_STOPPED_AUTO,
	RK_BT_DISC_START_FAILED,
	RK_BT_DISC_STOPPED_BY_USER,
} RK_BT_DISCOVERY_STATE;

typedef struct {
	Ble_Uuid_Type_t server_uuid;
	Ble_Uuid_Type_t chr_uuid[12];
	uint8_t chr_cnt;
	const char *ble_name;
	uint8_t advData[256];
	uint8_t advDataLen;
	uint8_t respData[256];
	uint8_t respDataLen;
	uint8_t advDataType;
	//AdvDataKgContent adv_kg;
	char le_random_addr[6];
	/* recevice data */
	void (*cb_ble_recv_fun)(const char *uuid, char *data, int len);
	/* full data */
	void (*cb_ble_request_data)(const char *uuid);
} RkBleContent;

typedef struct {
	RkBleContent ble_content;
	const char *bt_name;
	const char *bt_addr;
} RkBtContent;

typedef struct paired_dev RkBtPraiedDevice;

typedef void (*RK_BT_STATE_CALLBACK)(RK_BT_STATE state);
typedef void (*RK_BT_BOND_CALLBACK)(const char *bd_addr, const char *name, RK_BT_BOND_STATE state);
typedef void (*RK_BT_DISCOVERY_CALLBACK)(RK_BT_DISCOVERY_STATE state);
typedef void (*RK_BT_DEV_FOUND_CALLBACK)(const char *address, const char *name, unsigned int bt_class, int rssi);

void rk_bt_register_state_callback(RK_BT_STATE_CALLBACK cb);
void rk_bt_register_bond_callback(RK_BT_BOND_CALLBACK cb);
void rk_bt_register_discovery_callback(RK_BT_DISCOVERY_CALLBACK cb);
void rk_bt_register_dev_found_callback(RK_BT_DEV_FOUND_CALLBACK cb);
int rk_bt_init(RkBtContent *p_bt_content);
int rk_bt_deinit(void);
int rk_bt_is_connected(void);
int rk_bt_set_class(int value);
int rk_bt_set_sleep_mode(void);
int rk_bt_enable_reconnect(int value);
int rk_bt_start_discovery(unsigned int mseconds);
int rk_bt_cancel_discovery();
bool rk_bt_is_discovering();
void rk_bt_display_devices();
int rk_bt_pair_by_addr(char *addr);
int rk_bt_unpair_by_addr(char *addr);
int rk_bt_set_device_name(char *name);
int rk_bt_get_device_name(char *name, int len);
int rk_bt_get_device_addr(char *addr, int len);
int rk_bt_get_paired_devices(RkBtPraiedDevice **dev_list,int *count);
int rk_bt_free_paired_devices(RkBtPraiedDevice *dev_list);
void rk_bt_display_paired_devices();

/*INVALID = 0, SOURCE = 1, SINK = 2*/
int rk_bt_get_playrole_by_addr(char *addr);

#ifdef __cplusplus
}
#endif

#endif /* __BT_BASE_H__ */
