#!/bin/bash

set -e
DEPENDENCIES="udev libssl-dev libnl-3-dev libnl-genl-3-dev libreadline-dev"
$SCRIPTS_DIR/build_pkgs.sh $ARCH $SUITE "$DEPENDENCIES"
PKG=libwpa_client
PKGVER=wpa_supplicant-2.8
GZPKG=wpa_supplicant-2.8.tar.gz
if [ ! -e $DOWNLOAD_DIR/$GZPKG ];then
	wget -P $DOWNLOAD_DIR http://w1.fi/releases/$GZPKG
fi

if [ ! -e $BUILD_DIR/$PKG/.timestamp ];then
	tar -xzf $DOWNLOAD_DIR/$GZPKG -C $BUILD_DIR/$PKG
	mv $BUILD_DIR/$PKG/$PKGVER/* $BUILD_DIR/$PKG/
fi

cp $PACKAGE_DIR/$PKG/defconfig $BUILD_DIR/$PKG/wpa_supplicant/.config
cd $BUILD_DIR/$PKG/wpa_supplicant
make -j$RK_JOBS
make LIBDIR="/usr/lib" INCDIR="/usr/include" install
cd -


