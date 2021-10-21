#!/bin/bash

CMD=`realpath $0`
COMMON_DIR=`dirname $CMD`
TOP_DIR=$(realpath $COMMON_DIR/../../..)
BOARD_CONFIG=$TOP_DIR/device/rockchip/.BoardConfig.mk
source $BOARD_CONFIG
source $TOP_DIR/device/rockchip/common/Version.mk

function usage()
{
	echo "Usage: build.sh [OPTIONS]"
	echo "Available options:"
	echo "BoardConfig*.mk    -switch to specified board config"
	echo "uboot              -build uboot"
	echo "kernel             -build kernel"
	echo "dtb                -build kernel dtb"
	echo "modules            -build kernel modules"
	echo "rootfs             -build default rootfs, currently build buildroot as default"
	echo "buildroot          -build buildroot rootfs"
	echo "ramboot            -build ramboot image"
	echo "multi-npu_boot     -build boot image for multi-npu board"
	echo "yocto              -build yocto rootfs"
	echo "ubuntu             -build ubuntu rootfs"
	echo "debian             -build debian9 stretch rootfs"
	echo "distro             -build debian10 buster rootfs"
	echo "pcba               -build pcba"
	echo "recovery           -build recovery"
	echo "all                -build uboot, kernel, rootfs, recovery image"
	echo "cleanall           -clean uboot, kernel, rootfs, recovery"
	echo "firmware           -pack all the image we need to boot up system"
	echo "updateimg          -pack update image"
	echo "otapackage         -pack ab update otapackage image"
	echo "save               -save images, patches, commands used to debug"
	echo "allsave            -build all & firmware & updateimg & save"
	echo "temifw version     -build temi fireware and packaging it"
	echo ""
	echo "Default option is 'temifw'."
}

function build_uboot(){
	echo "============Start build uboot============"
	echo "TARGET_UBOOT_CONFIG=$RK_UBOOT_DEFCONFIG"
	echo "========================================="
	if [ -f u-boot/*_loader_*.bin ]; then
		rm u-boot/*_loader_*.bin
	fi
	cd u-boot && ./make.sh $RK_UBOOT_DEFCONFIG && cd -
	if [ $? -eq 0 ]; then
		echo "====Build uboot ok!===="
	else
		echo "====Build uboot failed!===="
		exit 1
	fi
}

function build_kernel(){
	echo "============Start build kernel============"
	echo "TARGET_ARCH          =$RK_ARCH"
	echo "TARGET_KERNEL_CONFIG =$RK_KERNEL_DEFCONFIG"
	echo "TARGET_KERNEL_DTS    =$RK_KERNEL_DTS"
	echo "=========================================="
	cd $TOP_DIR/kernel && make ARCH=$RK_ARCH $RK_KERNEL_DEFCONFIG && make ARCH=$RK_ARCH $RK_KERNEL_DTS.img -j$RK_JOBS && cd -
	if [ $? -eq 0 ]; then
		echo "====Build kernel ok!===="
	else
		echo "====Build kernel failed!===="
		exit 1
	fi
}

function build_dtb(){
	echo "============Start build kernel dtb============"
	devname="$RK_KERNEL_DTS"
	src_dts=$devname.dts
	pre_dts=/tmp/$devname.pre.dts
	dec_dts=/tmp/$devname.dec.dts
	dst_dtb=$devname.dtb
	dtc_cmd=$TOP_DIR/kernel/scripts/dtc/dtc
	echo "Preprocessed  dts: $pre_dts"
	echo "Decompilation dts: $dec_dts"

	cd $TOP_DIR/kernel/arch/arm64/boot/dts/rockchip
	cpp -nostdinc -I$TOP_DIR/kernel/include -undef -x assembler-with-cpp $src_dts > $pre_dts
	$dtc_cmd -O dtb -b 0 -o $dst_dtb $pre_dts
	$dtc_cmd -I dtb -O dts $dst_dtb -o $dec_dts
	cd - > /dev/null
	if [ $? -eq 0 ]; then
		echo "====Build dtb ok!===="
	else
		echo "====Build dtb failed!===="
		exit 1
	fi
}

function build_modules(){
	echo "============Start build kernel modules============"
	echo "TARGET_ARCH          =$RK_ARCH"
	echo "TARGET_KERNEL_CONFIG =$RK_KERNEL_DEFCONFIG"
	echo "=================================================="
	cd $TOP_DIR/kernel && make ARCH=$RK_ARCH $RK_KERNEL_DEFCONFIG && make ARCH=$RK_ARCH modules -j$RK_JOBS && cd -
	if [ $? -eq 0 ]; then
		echo "====Build modules ok!===="
	else
		echo "====Build modules failed!===="
		exit 1
	fi
}

function build_buildroot(){
	echo "==========Start build buildroot=========="
	echo "TARGET_BUILDROOT_CONFIG=$RK_CFG_BUILDROOT"
	echo "========================================="
	/usr/bin/time -f "you take %E to build builroot" $COMMON_DIR/mk-buildroot.sh $BOARD_CONFIG
	if [ $? -eq 0 ]; then
		echo "====Build buildroot ok!===="
	else
		echo "====Build buildroot failed!===="
		exit 1
	fi
}

function build_ramboot(){
	echo "=========Start build ramboot========="
	echo "TARGET_RAMBOOT_CONFIG=$RK_CFG_RAMBOOT"
	echo "====================================="
	/usr/bin/time -f "you take %E to build ramboot" $COMMON_DIR/mk-ramdisk.sh ramboot.img $RK_CFG_RAMBOOT
	if [ $? -eq 0 ]; then
		echo "====Build ramboot ok!===="
	else
		echo "====Build ramboot failed!===="
		exit 1
	fi
}

function build_multi-npu_boot(){
	if [ -z "$RK_MULTINPU_BOOT" ]; then
		echo "=========Please set 'RK_MULTINPU_BOOT=y' in BoardConfig.mk========="
		exit 1
	fi
	echo "=========Start build multi-npu boot========="
	echo "TARGET_RAMBOOT_CONFIG=$RK_CFG_RAMBOOT"
	echo "====================================="
	/usr/bin/time -f "you take %E to build multi-npu boot" $COMMON_DIR/mk-multi-npu_boot.sh
	if [ $? -eq 0 ]; then
		echo "====Build multi-npu boot ok!===="
	else
		echo "====Build multi-npu boot failed!===="
		exit 1
	fi
}

function build_yocto(){
	if [ -z "$RK_YOCTO_MACHINE" ]; then
		echo "This board doesn't support yocto!"
		exit 1
	fi

	echo "=========Start build ramboot========="
	echo "TARGET_MACHINE=$RK_YOCTO_MACHINE"
	echo "====================================="

	cd yocto
	ln -sf $RK_YOCTO_MACHINE.conf build/conf/local.conf
	source oe-init-build-env
	cd ..
	bitbake core-image-minimal -r conf/include/rksdk.conf

	if [ $? -eq 0 ]; then
		echo "====Build yocto ok!===="
	else
		echo "====Build yocto failed!===="
		exit 1
	fi
}

function build_ubuntu(){
	cd ubuntu

	# TODO: 

	cd ..
	if [ $? -eq 0 ]; then
		echo "====Build Ubuntu ok!===="
	else
		echo "====Build Ubuntu failed!===="
		exit 1
	fi
}

function build_debian(){
	cd debian

	if [ "$RK_ARCH" == "arm" ]; then
		ARCH=armhf
	fi
	if [ "$RK_ARCH" == "arm64" ]; then
		ARCH=arm64
	fi

	if [ ! -e linaro-stretch-alip-*.tar.gz ]; then
		echo "\033[36m Run mk-base-debian.sh first \033[0m"
		RELEASE=stretch TARGET=desktop ARCH=$ARCH ./mk-base-debian.sh
	fi

	VERSION=debug ARCH=$ARCH ./mk-rootfs-stretch.sh

	./mk-image.sh
	cd ..
	if [ $? -eq 0 ]; then
		echo "====Build Debian9 ok!===="
	else
		echo "====Build Debian9 failed!===="
		exit 1
	fi
}

function build_distro(){
	echo "===========Start build debian==========="
	echo "TARGET_ARCH=$RK_ARCH"
	echo "RK_DISTRO_DEFCONFIG=$RK_DISTRO_DEFCONFIG"
	echo "========================================"
	cd distro && make $RK_DISTRO_DEFCONFIG && /usr/bin/time -f "you take %E to build debian" $TOP_DIR/distro/make.sh && cd -
	if [ $? -eq 0 ]; then
		echo "====Build debian ok!===="
	else
		echo "====Build debian failed!===="
		exit 1
	fi
}

function build_rootfs(){
	rm -f $RK_ROOTFS_IMG

	case "$1" in
		yocto)
			build_yocto
			ROOTFS_IMG=yocto/build/tmp/deploy/images/$RK_YOCTO_MACHINE/rootfs.img
			;;
		debian)
			build_debian
			ROOTFS_IMG=debian/linaro-rootfs.img
			;;
		distro)
			build_distro
			ROOTFS_IMG=rootfs/linaro-rootfs.img
			;;
		ubuntu)
			build_ubuntu
			ROOTFS_IMG=ubuntu/ubuntu-rootfs.img
			;;
		*)
			build_buildroot
			ROOTFS_IMG=buildroot/output/$RK_CFG_BUILDROOT/images/rootfs.$RK_ROOTFS_TYPE
			;;
	esac

	[ -z "$ROOTFS_IMG" ] && return

	if [ ! -f "$ROOTFS_IMG" ]; then
		echo "$ROOTFS_IMG not generated?"
	else
		mkdir -p ${RK_ROOTFS_IMG%/*}
		ln -rsf $TOP_DIR/$ROOTFS_IMG $RK_ROOTFS_IMG
	fi
}

function build_recovery(){
	echo "==========Start build recovery=========="
	echo "TARGET_RECOVERY_CONFIG=$RK_CFG_RECOVERY"
	echo "========================================"
	/usr/bin/time -f "you take %E to build recovery" $COMMON_DIR/mk-ramdisk.sh recovery.img $RK_CFG_RECOVERY
	if [ $? -eq 0 ]; then
		echo "====Build recovery ok!===="
	else
		echo "====Build recovery failed!===="
		exit 1
	fi
}

function build_pcba(){
	echo "==========Start build pcba=========="
	echo "TARGET_PCBA_CONFIG=$RK_CFG_PCBA"
	echo "===================================="
	/usr/bin/time -f "you take %E to build pcba" $COMMON_DIR/mk-ramdisk.sh pcba.img $RK_CFG_PCBA
	if [ $? -eq 0 ]; then
		echo "====Build pcba ok!===="
	else
		echo "====Build pcba failed!===="
		exit 1
	fi
}

function build_all(){
	echo "============================================"
	echo "TARGET_ARCH=$RK_ARCH"
	echo "TARGET_PLATFORM=$RK_TARGET_PRODUCT"
	echo "TARGET_UBOOT_CONFIG=$RK_UBOOT_DEFCONFIG"
	echo "TARGET_KERNEL_CONFIG=$RK_KERNEL_DEFCONFIG"
	echo "TARGET_KERNEL_DTS=$RK_KERNEL_DTS"
	echo "TARGET_BUILDROOT_CONFIG=$RK_CFG_BUILDROOT"
	echo "TARGET_RECOVERY_CONFIG=$RK_CFG_RECOVERY"
	echo "TARGET_PCBA_CONFIG=$RK_CFG_PCBA"
	echo "TARGET_RAMBOOT_CONFIG=$RK_CFG_RAMBOOT"
	echo "============================================"
	build_uboot
	build_kernel
	build_rootfs ${RK_ROOTFS_SYSTEM:-buildroot}
	build_recovery
	build_ramboot
}

function build_cleanall(){
	echo "clean uboot, kernel, rootfs, recovery"
	cd $TOP_DIR/u-boot/ && make distclean && cd -
	cd $TOP_DIR/kernel && make distclean && cd -
	rm -rf $TOP_DIR/buildroot/output
	rm -rf $TOP_DIR/yocto/build
	rm -rf $TOP_DIR/distro/output
	rm -rf $TOP_DIR/debian/binary
}

function build_firmware(){
	./mkfirmware.sh $BOARD_CONFIG
	if [ $? -eq 0 ]; then
		echo "Make image ok!"
	else
		echo "Make image failed!"
		exit 1
	fi
}

function build_updateimg(){
	IMAGE_PATH=$TOP_DIR/rockdev
	PACK_TOOL_DIR=$TOP_DIR/tools/linux/Linux_Pack_Firmware
	if [ "$RK_LINUX_AB_ENABLE"x = "true"x ];then
		echo "Make Linux a/b update.img."
		build_otapackage
		source_package_file_name=`ls -lh $PACK_TOOL_DIR/rockdev/package-file | awk -F ' ' '{print $NF}'`
		cd $PACK_TOOL_DIR/rockdev && ln -fs "$source_package_file_name"-ab package-file && ./mkupdate.sh && cd -
		mv $PACK_TOOL_DIR/rockdev/update.img $IMAGE_PATH/update_ab.img
		cd $PACK_TOOL_DIR/rockdev && ln -fs $source_package_file_name package-file && cd -
		if [ $? -eq 0 ]; then
			echo "Make Linux a/b update image ok!"
		else
			echo "Make Linux a/b update image failed!"
			exit 1
		fi

	else
		echo "Make update.img"
		cd $PACK_TOOL_DIR/rockdev && ./mkupdate.sh && cd -
		mv $PACK_TOOL_DIR/rockdev/update.img $IMAGE_PATH
		if [ $? -eq 0 ]; then
			echo "Make update image ok!"
		else
			echo "Make update image failed!"
			exit 1
		fi
	fi
}

function build_otapackage(){
	IMAGE_PATH=$TOP_DIR/rockdev
	PACK_TOOL_DIR=$TOP_DIR/tools/linux/Linux_Pack_Firmware

	echo "Make ota ab update.img"
	source_package_file_name=`ls -lh $PACK_TOOL_DIR/rockdev/package-file | awk -F ' ' '{print $NF}'`
	cd $PACK_TOOL_DIR/rockdev && ln -fs "$source_package_file_name"-ota package-file && ./mkupdate.sh && cd -
	mv $PACK_TOOL_DIR/rockdev/update.img $IMAGE_PATH/update_ota.img
	cd $PACK_TOOL_DIR/rockdev && ln -fs $source_package_file_name package-file && cd -
	if [ $? -eq 0 ]; then
		echo "Make update ota ab image ok!"
	else
		echo "Make update ota ab image failed!"
		exit 1
	fi
}

function build_save(){
	IMAGE_PATH=$TOP_DIR/rockdev
	DATE=$(date  +%Y%m%d.%H%M)
	STUB_PATH=Image/"$RK_KERNEL_DTS"_"$DATE"_RELEASE_TEST
	STUB_PATH="$(echo $STUB_PATH | tr '[:lower:]' '[:upper:]')"
	export STUB_PATH=$TOP_DIR/$STUB_PATH
	export STUB_PATCH_PATH=$STUB_PATH/PATCHES
	mkdir -p $STUB_PATH

	#Generate patches
	$TOP_DIR/.repo/repo/repo forall -c "$TOP_DIR/device/rockchip/common/gen_patches_body.sh"

	#Copy stubs
	$TOP_DIR/.repo/repo/repo manifest -r -o $STUB_PATH/manifest_${DATE}.xml
	mkdir -p $STUB_PATCH_PATH/kernel
	cp $TOP_DIR/kernel/.config $STUB_PATCH_PATH/kernel
	cp $TOP_DIR/kernel/vmlinux $STUB_PATCH_PATH/kernel
	mkdir -p $STUB_PATH/IMAGES/
	cp $IMAGE_PATH/* $STUB_PATH/IMAGES/

	#Save build command info
	echo "UBOOT:  defconfig: $RK_UBOOT_DEFCONFIG" >> $STUB_PATH/build_cmd_info
	echo "KERNEL: defconfig: $RK_KERNEL_DEFCONFIG, dts: $RK_KERNEL_DTS" >> $STUB_PATH/build_cmd_info
	echo "BUILDROOT: $RK_CFG_BUILDROOT" >> $STUB_PATH/build_cmd_info
}

function build_allsave(){
	build_all
	build_firmware
	build_updateimg
	build_save
}

function build_temifw() {
	echo "Start make temi fireware"
	
	build_uboot
	build_kernel
	
	IMAGE_PATH=$TOP_DIR/rockdev
	IMAGE_NAME=new_update.img

	PACK_TOOL_DIR=$TOP_DIR/tools/linux/Linux_Pack_Firmware
	cd $PACK_TOOL_DIR/temidev && ./pack.sh $IMAGE_NAME && cd -
	mv $PACK_TOOL_DIR/temidev/$IMAGE_NAME $IMAGE_PATH/$IMAGE_NAME
	if [ $? -eq 0 ]; then
		echo "Make update image ok!"
		echo "Please check [$IMAGE_PATH/$IMAGE_NAME]"
	else
		echo "Make update image failed!"
		exit 1
	fi

	DATE=$(date  +%Y%m%d.%H%M)
	VERSION=v0
	[ -n "$1" ] && VERSION="$1"
	#STUB_PATH=Image/"$RK_KERNEL_DTS"_"$DATE"_RELEASE
	STUB_PATH=Image/"TEMI-RK3399-LINUX"_"$DATE"_RELEASE_$VERSION
	STUB_PATH="$(echo $STUB_PATH | tr '[:lower:]' '[:upper:]')"
	export STUB_PATH=$TOP_DIR/$STUB_PATH
	export STUB_PATCH_PATH=$STUB_PATH/PATCHES
	mkdir -p $STUB_PATH/IMAGES
	mkdir -p $STUB_PATH/TOOLS/{Linux_Repack_Tool,Linux_Upgrade_Tool,Windows_Upgrade_Tool}
	rsync -vaL $PACK_TOOL_DIR/temidev/{bin,pack.sh,unpack.sh,Readme.md} $STUB_PATH/TOOLS/Linux_Repack_Tool
	echo "remove upgrade_tool's log" && sudo rm -rf $TOP_DIR/tools/linux/Linux_Upgrade_Tool/Linux_Upgrade_Tool/log
	rsync -vaL $TOP_DIR/tools/linux/Linux_Upgrade_Tool/Linux_Upgrade_Tool/*        $STUB_PATH/TOOLS/Linux_Upgrade_Tool
	rsync -vaL $TOP_DIR/tools/windows/AndroidTool/AndroidTool_Release/*            $STUB_PATH/TOOLS/Windows_Upgrade_Tool
	cp $IMAGE_PATH/{MiniLoaderAll.bin,parameter.txt,uboot.img,trust.img,boot.img}  $STUB_PATH/IMAGES
	mv $IMAGE_PATH/$IMAGE_NAME $STUB_PATH/
	echo "How to burn update.img (Test under Ubuntu, ps: Only first burn need to step 1 and 2)" >> $STUB_PATH/Readme
	echo "1. sudo ./TOOLS/Linux_Upgrade_Tool/upgrade_tool_v1.24 ef $IMAGE_NAME" >> $STUB_PATH/Readme
	echo "2. sudo ./TOOLS/Linux_Upgrade_Tool/upgrade_tool       ef $IMAGE_NAME" >> $STUB_PATH/Readme
	echo "3. sudo ./TOOLS/Linux_Upgrade_Tool/upgrade_tool       uf $IMAGE_NAME" >> $STUB_PATH/Readme
	mkdir -p $STUB_PATCH_PATH/{u-boot,kernel}
	cp $TOP_DIR/u-boot/.config    $STUB_PATCH_PATH/u-boot
	cp $TOP_DIR/u-boot/u-boot     $STUB_PATCH_PATH/u-boot
	cp $TOP_DIR/u-boot/System.map $STUB_PATCH_PATH/u-boot
	cp $TOP_DIR/kernel/.config    $STUB_PATCH_PATH/kernel
	cp $TOP_DIR/kernel/vmlinux    $STUB_PATCH_PATH/kernel
	cp $TOP_DIR/kernel/System.map $STUB_PATCH_PATH/kernel
	#Save build command info
	echo "UBOOT:  defconfig: $RK_UBOOT_DEFCONFIG" >> $STUB_PATH/build_cmd_info
	echo "KERNEL: defconfig: $RK_KERNEL_DEFCONFIG, dts: $RK_KERNEL_DTS" >> $STUB_PATH/build_cmd_info

	cd $STUB_PATH
	md5sum $IMAGE_NAME > ${IMAGE_NAME}.md5sum
	7z a PRODUCTION_FIREWARE.7z Readme ${IMAGE_NAME}* TOOLS/{Linux_Upgrade_Tool,Windows_Upgrade_Tool}
	fwsum=`md5sum PRODUCTION_FIREWARE.7z | cut -d ' ' -f1`
	mv PRODUCTION_FIREWARE.7z PRODUCTION_FIREWARE_${VERSION}_${fwsum}.7z
	rm ${IMAGE_NAME}.md5sum
	mv ${IMAGE_NAME} $IMAGE_PATH/${IMAGE_NAME}.${VERSION}
	cd -

	echo "make temi fireware ok!"
}

#=========================
# build targets
#=========================

if echo $@|grep -wqE "help|-h"; then
	usage
	exit 0
fi

OPTIONS="$@"
for option in ${OPTIONS:-temifw}; do
	echo "processing option: $option"
	case $option in
		BoardConfig*.mk)
			CONF=$TOP_DIR/device/rockchip/$RK_TARGET_PRODUCT/$option
			echo "switching to board: $CONF"
			if [ ! -f $CONF ]; then
				echo "not exist!"
				exit 1
			fi

			ln -sf $CONF $BOARD_CONFIG
			;;
		buildroot|ubuntu|debian|distro|yocto)
			build_rootfs $option
			;;
		recovery)
			build_kernel
			;&
		temifw)
			[ $# -ne 2 ] && echo "Parameter error!" && usage && exit 1
			shift 1
			build_temifw $@
			break
			;;
		*)
			eval build_$option || usage
			;;
	esac
done
