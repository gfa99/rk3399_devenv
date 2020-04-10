#!/bin/bash

set -e
DEPENDENCIES="mpp gst-plugins-base"
$SCRIPTS_DIR/build_pkgs.sh $ARCH $SUITE "$DEPENDENCIES"
mkdir -p $BUILD_DIR/gstreamer-rockchip
cd $BUILD_DIR/gstreamer-rockchip
$TOP_DIR/external/gstreamer-rockchip/autogen.sh --srcdir=$TOP_DIR/external/gstreamer-rockchip --prefix=/usr --libdir=/usr/lib/$TOOLCHAIN --target=aarch64-linux-gnu --host=aarch64-linux-gnu --program-prefix= --disable-valgrind --disable-examples --disable-rkximage
make -j$RK_JOBS
make install
cd -
