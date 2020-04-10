#!/bin/bash

set -e

if [ $RK_ARCH = arm ];then
CAMERA_ENGINE_RKISP_LIB=lib32
elif [ $RK_ARCH = arm64 ];then
CAMERA_ENGINE_RKISP_LIB=lib64
fi

PKG=camera_engine_rkisp
RKafDir=$TARGET_DIR/usr/lib/rkisp/af
RKaeDir=$TARGET_DIR/usr/lib/rkisp/ae
RKawbDir=$TARGET_DIR/usr/lib/rkisp/awb

mkdir -p $BUILD_DIR/$PKG
mkdir -p $RKafDir
mkdir -p $RKaeDir
mkdir -p $RKawbDir
mkdir -p $TARGET_DIR/etc/iqfiles

cd $TOP_DIR/external/$PKG
make ARCH=$ARCH OUTDIR=$BUILD_DIR/$PKG/build -j$RK_JOBS
install -D -m 755 $DISTRO_DIR/package/camera_engine_rkisp/S40rkisp_3A $TARGET_DIR/etc/init.d/
install -D -m 755 $BUILD_DIR/$PKG/build/bin/rkisp_demo $TARGET_DIR/usr/bin/
install -D -m 755 $BUILD_DIR/$PKG/build/bin/rkisp_3A_server $TARGET_DIR/usr/bin/
install -D -m 644 $TOP_DIR/external/$PKG/iqfiles/*.xml $TARGET_DIR/etc/iqfiles/
install -D -m 644 $BUILD_DIR/$PKG/build/lib/librkisp.so $TARGET_DIR/usr/lib/

install -D -m 644 $TOP_DIR/external/$PKG/plugins/3a/rkiq/af/$CAMERA_ENGINE_RKISP_LIB/librkisp_af.so $RKafDir/
install -D -m 644 $TOP_DIR/external/$PKG/plugins/3a/rkiq/aec/$CAMERA_ENGINE_RKISP_LIB/librkisp_aec.so $RKaeDir/
install -D -m 644 $TOP_DIR/external/$PKG/plugins/3a/rkiq/awb/$CAMERA_ENGINE_RKISP_LIB/librkisp_awb.so $RKawbDir/
cd -
