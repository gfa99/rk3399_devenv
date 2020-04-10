#!/bin/bash

set -e
PKG=QLauncher
DEPENDENCIES="weston libqt5widgets5 libatomic1 qtwayland5"
$SCRIPTS_DIR/build_pkgs.sh $ARCH $SUITE "$DEPENDENCIES"
#QMAKE=/usr/bin/qmake
QMAKE=$TOP_DIR/buildroot/output/$RK_CFG_BUILDROOT/host/bin/qmake
#QMAKE=$BUILD_DIR/host-qmake-5.9.4/bin/qmake
mkdir -p $BUILD_DIR/$PKG
cd $BUILD_DIR/$PKG
$QMAKE $TOP_DIR/app/$PKG
make -j$RK_JOBS
install -m 0755 -D $BUILD_DIR/$PKG/$PKG $TARGET_DIR/usr/bin/$PKG
install -m 0755 -D $TOP_DIR/app/$PKG/S50launcher $TARGET_DIR/etc/init.d/S50launcher
cd -

