#!/bin/bash

set -e
source $OUTPUT_DIR/.config
BT_TTY=$BR2_PACKAGE_RKWIFIBT_BTUART
echo "$BR2_PACKAGE_RKWIFIBT_VENDOR"
if [ $BR2_PACKAGE_RKWIFIBT_VENDOR = "BROADCOM" ];then
	LIBDEVICEIOSO=libDeviceIo_broadcom.so
	DEVICEIO_BSA=broadcom_bsa
fi
if [ $BR2_PACKAGE_RKWIFIBT_VENDOR = "REALTEK" ];then
	LIBDEVICEIOSO=libDeviceIo_bluez.so
	DEPENDENCIES+="readline bluez5_utils libglib2 bluez-alsa"
fi
if [ $BR2_PACKAGE_RKWIFIBT_VENDOR = "CYPRESS" ];then
	LIBDEVICEIOSO=libDeviceIo_cypress.so
	DEVICEIO_BSA=cypress_bsa
fi
if [ $BR2_PACKAGE_RKWIFIBT_VENDOR = "ROCKCHIP" ];then
	LIBDEVICEIOSO=libDeviceIo_fake.so
	DEVICEIO_BSA=fake
fi
echo "$DEPENDENCIES"
DEPENDENCIES+=" wpasupplicant libasound2-dev libwpa_client"
echo "$DEPENDENCIES"
$SCRIPTS_DIR/build_pkgs.sh $ARCH $SUITE "$DEPENDENCIES"

if [ $RK_ARCH = arm64 ];then
	DEVICEIOARCH=lib64
	BSAARCH=arm64
elif [ $RK_ARCH = arm ];then
	DEVICEIOARCH=lib32
	BSAARCH=arm
fi

install -m 0755 -D $TOP_DIR/external/deviceio_release/bsa_bt_sink.sh $TARGET_DIR/usr/bin/bsa_bt_sink.sh
install -m 0755 -D $TOP_DIR/external/deviceio_release/bsa_server.sh $TARGET_DIR/usr/bin/bsa_server.sh
install -m 0755 -D $TOP_DIR/external/deviceio_release/$DEVICEIO_BSA/$BSAARCH/libbsa.so $TARGET_DIR/usr/lib/libbsa.so
install -m 0755 -D $TOP_DIR/external/deviceio_release/$DEVICEIO_BSA/$BSAARCH/app_manager $TARGET_DIR/usr/bin/app_manager
install -m 0755 -D $TOP_DIR/external/deviceio_release/$DEVICEIO_BSA/$BSAARCH/bsa_server $TARGET_DIR/usr/bin/bsa_server
install -m 0755 -D $TOP_DIR/external/deviceio_release/$DEVICEIO_BSA/$BSAARCH/libbsa.so $TARGET_DIR/usr/lib/libbsa.so
install -m 0755 -D $TOP_DIR/external/deviceio_release/DeviceIO/$DEVICEIOARCH/$LIBDEVICEIOSO $TARGET_DIR/usr/lib/libDeviceIo.so
install -m 0755 -D $TOP_DIR/external/deviceio_release/DeviceIO/$DEVICEIOARCH/$LIBDEVICEIOSO $STAGING_DIR/usr/lib/libDeviceIo.so
sed -i "s/BT_TTY_DEV/\/dev\/$BT_TTY/g" $TARGET_DIR/usr/bin/bsa_server.sh

