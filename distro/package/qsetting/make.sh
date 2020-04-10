#!/bin/bash

set -e
DEPENDENCIES="weston libqt5widgets5 qtwayland5 libqt5quickwidgets5 deviceio_release"
$SCRIPTS_DIR/build_pkgs.sh $ARCH $SUITE "$DEPENDENCIES"
PKG=qsetting
#QMAKE=/usr/bin/qmake
QMAKE=$TOP_DIR/buildroot/output/$RK_CFG_BUILDROOT/host/bin/qmake
mkdir -p $BUILD_DIR/$PKG
cd $BUILD_DIR/$PKG
$QMAKE $TOP_DIR/app/$PKG
make -j$RK_JOBS CXXFLAGS+="-DRKDEVICEIO -I$TOP_DIR/external/deviceio_release/DeviceIO/include -L$TARGET_DIR/usr/lib" LFLAGS+=" -lbsa -lDeviceIo -lasound -L$TARGET_DIR/usr/lib"
mkdir -p $TARGET_DIR/usr/share/icon
cp $TOP_DIR/app/$PKG/icon_qsetting.png $TARGET_DIR/usr/share/icon/
mkdir -p $TARGET_DIR/usr/share/applications
install -m 0644 -D $TOP_DIR/app/$PKG/qsetting.desktop $TARGET_DIR/usr/share/applications/
install -m 0755 -D $BUILD_DIR/$PKG/qsetting $TARGET_DIR/usr/bin/qsetting
install -m 0755 -D $TOP_DIR/app/$PKG/S80wifireconnect $TARGET_DIR/etc/init.d/
cd -
