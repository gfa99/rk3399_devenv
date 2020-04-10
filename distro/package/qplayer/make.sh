#!/bin/bash

set -e
DEPENDENCIES="gstreamer-rockchip weston libqt5widgets5 libatomic1 qtwayland5 libqt5multimedia5 libqt5multimediawidgets5 libqt5multimediagsttools5"
$SCRIPTS_DIR/build_pkgs.sh $ARCH $SUITE "$DEPENDENCIES"
PKG=qplayer
#QMAKE=/usr/bin/qmake
QMAKE=$TOP_DIR/buildroot/output/$RK_CFG_BUILDROOT/host/bin/qmake
mkdir -p $BUILD_DIR/$PKG
cd $BUILD_DIR/$PKG
$QMAKE $TOP_DIR/app/$PKG
make -j$RK_JOBS
mkdir -p $TARGET_DIR/usr/share/icon
cp $TOP_DIR/app/$PKG/icon_player.png $TARGET_DIR/usr/share/icon/
mkdir -p $TARGET_DIR/usr/share/applications
install -m 0644 -D $TOP_DIR/app/$PKG/qplayer.desktop $TARGET_DIR/usr/share/applications/
install -m 0755 -D $BUILD_DIR/$PKG/qplayer $TARGET_DIR/usr/bin/qplayer
cd -

