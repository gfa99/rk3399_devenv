#!/bin/bash

set -e
DEPENDENCIES=""
$SCRIPTS_DIR/build_pkgs.sh $ARCH $SUITE "$DEPENDENCIES"
source $OUTPUT_DIR/.config
APP_PATH=3rdparty/embedded/bsa_examples/linux
mkdir -p $TARGET_DIR/etc/bsa_file
cp -r $TOP_DIR/external/broadcom_bsa/* $BUILD_DIR/broadcom_bsa/
BT_TTY=ttyS0
echo "BR2_PACKAGE_RKWIFIBT_BTUART:$BR2_PACKAGE_RKWIFIBT_BTUART"
if [ -n $BR2_PACKAGE_RKWIFIBT_BTUART ];then
        BT_TTY=$BR2_PACKAGE_RKWIFIBT_BTUART
fi

APP="app_manager app_av app_avk app_ble app_dg app_hl app_hs app_tm app_tm app_socket app_hd app_hh app_ble_wifi_introducer"
echo $APP
export TARGET_LINK_FLAGS=$LDFLAGS
for ff in $APP;
do
	make -C $BUILD_DIR/broadcom_bsa/$APP_PATH/$ff/build CPU=$RK_ARCH ARMGCC="$GCC --sysroot=$SYSROOT_DIR" BSASHAREDLIB=TRUE;
	install -m 755 -D $BUILD_DIR/broadcom_bsa/$APP_PATH/$ff/build/$RK_ARCH/$ff $TARGET_DIR/usr/bin/$ff;
done

install -m 755 -D $TOP_DIR/external/broadcom_bsa/server/$RK_ARCH/bsa_server $TARGET_DIR/usr/bin/bsa_server
install -m 644 -D $TOP_DIR/external/broadcom_bsa/$APP_PATH/libbsa/build/$RK_ARCH/sharedlib/libbsa.so $TARGET_DIR/usr/lib/libbsa.so
install -m 644 -D $TOP_DIR/external/broadcom_bsa/test_files/av/8k8bpsMono.wav $TARGET_DIR/etc/bsa_file/
install -m 644 -D $TOP_DIR/external/broadcom_bsa/test_files/av/8k16bpsStereo.wav $TARGET_DIR/etc/bsa_file/
install -m 755 -D $TOP_DIR/external/broadcom_bsa/bsa_bt_hfp.sh $TARGET_DIR/usr/bin/bsa_bt_hfp.sh
install -m 755 -D $TOP_DIR/external/broadcom_bsa/bsa_server.sh $TARGET_DIR/usr/bin/
install -m 755 -D $TOP_DIR/external/broadcom_bsa/bsa_bt_sink.sh $TARGET_DIR/usr/bin/bsa_bt_sink.sh
install -m 755 -D $TOP_DIR/external/broadcom_bsa/bsa_bt_source.sh $TARGET_DIR/usr/bin/
install -m 755 -D $TOP_DIR/external/broadcom_bsa/bsa_ble_wifi_introducer.sh $TARGET_DIR/usr/bin/bsa_ble_wifi_introducer.sh
echo $BTFIRMWARE
sed -i "s/BTFIRMWARE_PATH/\/system\/etc\/firmware\/$BTFIRMWARE/g" $TARGET_DIR/usr/bin/bsa_bt_hfp.sh
sed -i "s/BTFIRMWARE_PATH/\/system\/etc\/firmware\/$BTFIRMWARE/g" $TARGET_DIR/usr/bin/bsa_server.sh
sed -i "s/BT_TTY_DEV/\/dev\/$BT_TTY/g" $TARGET_DIR/usr/bin/bsa_server.sh
sed -i "s/BTFIRMWARE_PATH/\/system\/etc\/firmware\/$BTFIRMWARE/g" $TARGET_DIR/usr/bin/bsa_bt_sink.sh
sed -i "s/BTFIRMWARE_PATH/\/system\/etc\/firmware\/$BTFIRMWARE/g" $TARGET_DIR/usr/bin/bsa_bt_source.sh
sed -i "s/BTFIRMWARE_PATH/\/system\/etc\/firmware\/$BTFIRMWARE/g" $TARGET_DIR/usr/bin/bsa_ble_wifi_introducer.sh

