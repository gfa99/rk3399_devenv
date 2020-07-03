#!/bin/bash

set -e
DEPENDENCIES="libglib2.0-dev libgstreamer1.0-0"
$SCRIPTS_DIR/build_pkgs.sh $ARCH $SUITE "$DEPENDENCIES"
PKG=gstreamer
VERSION=1.14.4
if [ ! -e $DOWNLOAD_DIR/$PKG-$VERSION.tar.xz ];then
	wget -P $DOWNLOAD_DIR https://gstreamer.freedesktop.org/src/gstreamer/$PKG-$VERSION.tar.xz
fi

if [ ! -d $BUILD_DIR/$PKG/$PKG-$VERSION ];then
	tar -xf $DOWNLOAD_DIR/$PKG-$VERSION.tar.xz -C $BUILD_DIR/$PKG
	mv $BUILD_DIR/$PKG/$PKG-$VERSION/* $BUILD_DIR/$PKG/
fi

cd $BUILD_DIR/$PKG
./configure --target=aarch64-linux-gnu --host=aarch64-linux-gnu --prefix=/usr --libdir=/usr/lib/$TOOLCHAIN --program-prefix= --disable-gtk-doc --disable-gtk-doc-html --disable-dependency-tracking --disable-nls --disable-static --enable-shared  --disable-examples --disable-tests --disable-failing-tests --disable-valgrind --disable-benchmarks --disable-introspection --disable-check
make -j$RK_JOBS
make install
$SCRIPTS_DIR/fixlibtool.sh $TARGET_DIR $TARGET_DIR
