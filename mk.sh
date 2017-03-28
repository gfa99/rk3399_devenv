#!/bin/bash
#
# Description	: Android Build Script.
# Authors		: jianjun jiang - jerryjianjun@gmail.com
# Version		: 2.00
# Notes			: None
#

#
# Some Directories
#
BS_DIR_TOP=$(cd `dirname $0` ; pwd)
BS_DIR_TOOLS=${BS_DIR_TOP}/tools
BS_DIR_OUTPUT=${BS_DIR_TOP}/output
BS_DIR_UBOOT=${BS_DIR_TOP}/u-boot
BS_DIR_KERNEL=${BS_DIR_TOP}/kernel

#
# Target Config
#
BS_CONFIG_BOOTLOADER_UBOOT=x3399_defconfig
BS_CONFIG_KERNEL=x3399_defconfig
BS_CONFIG_KERNEL_DTB=x3399-development-board.img

setup_environment()
{
	LANG=C
	cd ${BS_DIR_TOP};
	mkdir -p ${BS_DIR_OUTPUT} || return 1
}

build_bootloader_uboot()
{
	# Compiler uboot
	cd ${BS_DIR_UBOOT} || return 1
	make ARCHV=aarch64 distclean || return 1
	make ARCHV=aarch64 ${BS_CONFIG_BOOTLOADER_UBOOT} || return 1
	make ARCHV=aarch64 -j${threads} || return 1

	# Copy bootloader to release directory
	cp -v ${BS_DIR_UBOOT}/trust.img ${BS_DIR_OUTPUT}
	cp -v ${BS_DIR_UBOOT}/*MiniLoaderAll_*.bin ${BS_DIR_OUTPUT}
	cp -v ${BS_DIR_UBOOT}/uboot.img ${BS_DIR_OUTPUT}

	return 0
}

build_kernel()
{
	# Compiler kernel
	cd ${BS_DIR_KERNEL} || return 1
	make ARCH=arm64 ${BS_CONFIG_KERNEL} || return 1
	make -j${threads} ARCH=arm64 Image || return 1
	make -j${threads} ARCH=arm64 ${BS_CONFIG_KERNEL_DTB} || return 1

	# Copy kernel to release directory
	cp -v ${BS_DIR_KERNEL}/resource.img ${BS_DIR_OUTPUT}
	cp -v ${BS_DIR_KERNEL}/kernel.img ${BS_DIR_OUTPUT}

	return 0
}

threads=1
uboot=no
kernel=no

if [ -z $1 ]; then
	uboot=yes
	kernel=yes
fi

while [ "$1" ]; do
    case "$1" in
	-j=*)
		x=$1
		threads=${x#-j=}
		;;
	-u|--uboot)
		uboot=yes
	    ;;
	-k|--kernel)
	    kernel=yes
	    ;;
	-a|--all)
		uboot=yes
		kernel=yes
	    ;;
	-h|--help)
	    cat >&2 <<EOF
Usage: build.sh [OPTION]
Build script for compile the source of telechips project.

  -j=n                 using n threads when building source project (example: -j=16)
  -u, --uboot          build bootloader uboot from source
  -k, --kernel         build kernel from source
  -a, --all            build all, include anything
  -h, --help           display this help and exit
EOF
	    exit 0
	    ;;
	*)
	    echo "build.sh: Unrecognised option $1" >&2
	    exit 1
	    ;;
    esac
    shift
done

setup_environment || exit 1

if [ "${uboot}" = yes ]; then
	build_bootloader_uboot || exit 1
fi

if [ "${kernel}" = yes ]; then
	build_kernel || exit 1
fi

exit 0
