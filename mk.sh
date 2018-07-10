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
	cp -v ${BS_DIR_UBOOT}/rk3399_loader_*.bin ${BS_DIR_OUTPUT}
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

build_update()
{
	cd ${BS_DIR_RELEASE} || return 1
	
	# Make update-debian.img
	echo "create update-debian.img..."
	ln -sf ${BS_DIR_OUTPUT}/debian-rootfs.img ${BS_DIR_OUTPUT}/linux-rootfs.img || return 1;
	cp -av ${BS_DIR_TOOLS}/package-file ${BS_DIR_OUTPUT}/package-file || return 1;
	${BS_DIR_TOOLS}/afptool -pack ${BS_DIR_OUTPUT}/ ${BS_DIR_OUTPUT}/temp.img || return 1;
	${BS_DIR_TOOLS}/rkImageMaker -RK330C ${BS_DIR_OUTPUT}/rk3399_loader_v1.12.109.bin ${BS_DIR_OUTPUT}/temp.img ${BS_DIR_OUTPUT}/update-debian.img -os_type:androidos || return 1;
	rm -fr ${BS_DIR_OUTPUT}/temp.img || return 1;

	# Make update-ubuntu.img
	echo "create update-ubuntu.img..."
	ln -sf ${BS_DIR_OUTPUT}/ubuntu-rootfs.img ${BS_DIR_OUTPUT}/linux-rootfs.img || return 1;
	cp -av ${BS_DIR_TOOLS}/package-file ${BS_DIR_OUTPUT}/package-file || return 1;
	${BS_DIR_TOOLS}/afptool -pack ${BS_DIR_OUTPUT}/ ${BS_DIR_OUTPUT}/temp.img || return 1;
	${BS_DIR_TOOLS}/rkImageMaker -RK330C ${BS_DIR_OUTPUT}/rk3399_loader_v1.12.109.bin ${BS_DIR_OUTPUT}/temp.img ${BS_DIR_OUTPUT}/update-ubuntu.img -os_type:androidos || return 1;
	rm -fr ${BS_DIR_OUTPUT}/temp.img || return 1;

	return 0
}

copy_other_files()
{
	cd ${BS_DIR_TOP} || return 1

	cp -av ${BS_DIR_TOOLS}/parameter.txt ${BS_DIR_OUTPUT} || return 1;
	if [ ! -f ${BS_DIR_OUTPUT}/debian-rootfs.img ]; then
		tar xvf ${BS_DIR_TOOLS}/debian-rootfs.img.tar.bz2 -C ${BS_DIR_OUTPUT}/
	fi
	if [ ! -f ${BS_DIR_OUTPUT}/ubuntu-rootfs.img ]; then
		tar xvf ${BS_DIR_TOOLS}/ubuntu-rootfs.img.tar.bz2 -C ${BS_DIR_OUTPUT}/
	fi
	return 0
}

threads=1
uboot=no
kernel=no
update=no

if [ -z $1 ]; then
	uboot=yes
	kernel=yes
	update=yes
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
	-U|--update)
		update=yes
	    ;;
	-a|--all)
		uboot=yes
		kernel=yes
		update=yes
	    ;;
	-h|--help)
	    cat >&2 <<EOF
Usage: build.sh [OPTION]
Build script for compile the source of telechips project.

  -j=n                 using n threads when building source project (example: -j=16)
  -u, --uboot          build bootloader uboot from source
  -k, --kernel         build kernel from source
  -U, --update         build update file
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
copy_other_files || exit 1

if [ "${uboot}" = yes ]; then
	build_bootloader_uboot || exit 1
fi

if [ "${kernel}" = yes ]; then
	build_kernel || exit 1
fi

if [ "${update}" = yes ]; then
	build_update || exit 1
fi

exit 0
