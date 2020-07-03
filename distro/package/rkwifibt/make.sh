#!/bin/bash

set -e
DEPENDENCIES="wpasupplicant dhcpcd5 ntp ntpdate"
$SCRIPTS_DIR/build_pkgs.sh $ARCH $SUITE "$DEPENDENCIES"
source $OUTPUT_DIR/.config
mkdir -p $BUILD_DIR/rkwifibt/realtek/rtk_hciattach
cp -r $TOP_DIR/external/rkwifibt/realtek/rtk_hciattach $BUILD_DIR/rkwifibt/realtek/
cp -r $TOP_DIR/external/rkwifibt/realtek/bluetooth_uart_driver $BUILD_DIR/rkwifibt/realtek/
mkdir -p $TARGET_DIR/system/lib/modules $TARGET_DIR/system/etc/firmware $TARGET_DIR/lib/firmware $TARGET_DIR/usr/lib/modules $TARGET_DIR/lib/firmware/rtlbt
BT_TTY=ttyS0
echo "BR2_PACKAGE_RKWIFIBT_BTUART:$BR2_PACKAGE_RKWIFIBT_BTUART"
if [ -n $BR2_PACKAGE_RKWIFIBT_BTUART ];then
        BT_TTY=$BR2_PACKAGE_RKWIFIBT_BTUART
fi

function build_rkwifibt(){
if [ -e $TOP_DIR/build.sh ];then
export LDFLAGS="--sysroot=$SYSROOT"
$TOP_DIR/build.sh modules
find $TOP_DIR/kernel/drivers/net/wireless/rockchip_wlan/* -name $BR2_PACKAGE_RKWIFIBT_WIFI_KO | xargs -n1 -i cp {} $TARGET_DIR/system/lib/modules/
fi

$GCC $TOP_DIR/external/rkwifibt/src/rk_wifi_init.c -o $TARGET_DIR/usr/bin/rk_wifi_init
}

function install_common(){
install -m 0755 -D $TOP_DIR/external/rkwifibt/wpa_supplicant.conf $TARGET_DIR/etc/
install -m 0755 -D $TOP_DIR/external/rkwifibt/dnsmasq.conf $TARGET_DIR/etc/
install -m 0755 -D $TOP_DIR/external/rkwifibt/wifi_start.sh $TARGET_DIR/usr/bin/
}

function install_broadcom(){
	build_rkwifibt
	install_common
	$GCC $TOP_DIR/external/rkwifibt/brcm_tools/brcm_patchram_plus1.c -o $TARGET_DIR/usr/bin/brcm_patchram_plus1
	$GCC $TOP_DIR/external/rkwifibt/brcm_tools/dhd_priv.c -o $TARGET_DIR/usr/bin/dhd_priv
	install -m 0755 -D $TOP_DIR/external/rkwifibt/S66load_wifi_modules $TARGET_DIR/etc/init.d/
	sed -i "s/BT_TTY_DEV/\/dev\/$BT_TTY/g" $TARGET_DIR/etc/init.d/S66load_wifi_modules
	install -m 0644 -D $TOP_DIR/external/rkwifibt/firmware/broadcom/$BR2_PACKAGE_RKWIFIBT_CHIPNAME/wifi/* $TARGET_DIR/system/etc/firmware/
	install -m 0755 -D $TOP_DIR/external/rkwifibt/bin/$RK_ARCH/* $TARGET_DIR/usr/bin/

	install -m 0644 -D $TOP_DIR/external/rkwifibt/firmware/broadcom/$BR2_PACKAGE_RKWIFIBT_CHIPNAME/bt/* $TARGET_DIR/system/etc/firmware/
	install -m 0755 -D $TOP_DIR/external/rkwifibt/bt_load_broadcom_firmware $TARGET_DIR/usr/bin/
	sed -i "s/BTFIRMWARE_PATH/\/system\/etc\/firmware\/$BR2_PACKAGE_RKWIFIBT_BT_FW/g" $TARGET_DIR/usr/bin/bt_load_broadcom_firmware
	sed -i "s/BT_TTY_DEV/\/dev\/$BT_TTY/g" $TARGET_DIR/usr/bin/bt_load_broadcom_firmware
	install -m 0755 -D $TARGET_DIR/usr/bin/bt_load_broadcom_firmware $TARGET_DIR/usr/bin/bt_pcba_test
	install -m 0755 -D $TARGET_DIR/usr/bin/bt_load_broadcom_firmware $TARGET_DIR/usr/bin/bt_init.sh
}

function install_realtek(){
        build_rkwifibt
        install_common
	make -C $BUILD_DIR/rkwifibt/realtek/rtk_hciattach/ CC=$GCC
	make -C $TOP_DIR/kernel/ M=$BUILD_DIR/rkwifibt/realtek/bluetooth_uart_driver ARCH=$RK_ARCH CROSS_COMPILE=$CROSS_COMPILE
	install -m 0755 -D $TOP_DIR/external/rkwifibt/bin/$RK_ARCH/rtwpriv $TARGET_DIR/usr/bin/
	install -m 0755 -D $TOP_DIR/external/rkwifibt/S66load_wifi_modules $TARGET_DIR/etc/init.d/
	install -m 0755 -D $TOP_DIR/external/rkwifibt/realtek/rtk_hciattach/rtk_hciattach $TARGET_DIR/usr/bin/rtk_hciattach
	install -m 0755 -D $TOP_DIR/external/rkwifibt/bin/$RK_ARCH/* $TARGET_DIR/usr/bin/
	install -m 0644 -D $TOP_DIR/external/rkwifibt/realtek/$BR2_PACKAGE_RKWIFIBT_CHIPNAME/* $TARGET_DIR/lib/firmware/rtlbt/
	install -m 0644 -D $TOP_DIR/external/rkwifibt/realtek/$BR2_PACKAGE_RKWIFIBT_CHIPNAME/mp_* $TARGET_DIR/lib/firmware/rtlbt/
	install -m 0644 -D $TOP_DIR/external/rkwifibt/realtek/$BR2_PACKAGE_RKWIFIBT_CHIPNAME/mp_* $TARGET_DIR/lib/firmware/
	install -m 0755 -D $TOP_DIR/external/rkwifibt/bt_realtek* $TARGET_DIR/usr/bin/
	install -m 0644 -D $TOP_DIR/external/rkwifibt/realtek/bluetooth_uart_driver/hci_uart.ko $TARGET_DIR/usr/lib/modules/hci_uart.ko
	install -m 0755 -D $TOP_DIR/external/rkwifibt/bt_load_rtk_firmware $TARGET_DIR/usr/bin/
	sed -i "s/BT_TTY_DEV/\/dev\/$BT_TTY/g" $TARGET_DIR/usr/bin/bt_load_rtk_firmware
	install -m 0755 -D $TARGET_DIR/usr/bin/bt_load_rtk_firmware $TARGET_DIR/usr/bin/bt_pcba_test
	install -m 0755 -D $TARGET_DIR/usr/bin/bt_load_rtk_firmware $TARGET_DIR/usr/bin/bt_init.sh
}

function install_rockchip(){
	install_common
	install -m 0644 -D $TOP_DIR/external/rkwifibt/firmware/rockchip/WIFI_FIRMWARE/rk912* $TARGET_DIR/lib/firmware/
	install -m 0755 -D $TOP_DIR/external/rkwifibt/S66load_wifi_rk912_modules $TARGET_DIR/etc/init.d/
}
echo $BR2_PACKAGE_RKWIFIBT_VENDOR
if [ $BR2_PACKAGE_RKWIFIBT_VENDOR = "BROADCOM" ];then
	install_broadcom
fi
if [ $BR2_PACKAGE_RKWIFIBT_VENDOR = "REALTEK" ];then
        install_realtek
fi
if [ $BR2_PACKAGE_RKWIFIBT_VENDOR = "CYPRESS" ];then
        install_broadcom
fi
if [ $BR2_PACKAGE_RKWIFIBT_VENDOR = "ROCKCHIP" ];then
        install_rockchip
fi
install -m 0755 -D $PACKAGE_DIR/rkwifibt/S41dhcpcd $TARGET_DIR/etc/init.d/
install -m 0755 -D $PACKAGE_DIR/rkwifibt/S69ntp $TARGET_DIR/etc/init.d/
install -m 0755 -D $PACKAGE_DIR/rkwifibt/watch_ntpd.sh $TARGET_DIR/usr/bin/

sed -i s/debian/cn/g $TARGET_DIR/etc/ntp.conf
