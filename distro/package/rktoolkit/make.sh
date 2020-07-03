#!/bin/bash

set -e
$GCC $TOP_DIR/external/rktoolkit/io.c --sysroot=$SYSROOT_DIR -I$SYSROOT_DIR/usr/include -I$SYSROOT_DIR/usr/include/$TOOLCHAIN -o $TARGET_DIR/usr/bin/io
$GCC $TOP_DIR/external/rktoolkit/update.c $TOP_DIR/external/rktoolkit/update_recv/update_recv.c --sysroot=$SYSROOT_DIR -I$SYSROOT_DIR/usr/include -I$SYSROOT_DIR/usr/include/$TOOLCHAIN -I$TOP_DIR/external/rktoolkit/update_recv/ -o $TARGET_DIR/usr/bin/update
