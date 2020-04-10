#!/usr/bin/env bash
###############################################################################
# Author  : Kevin Oscar<zrzxlfe@sina.com>
# Date    : 2019-04-03
# Version : v1.0
# Abstract: rockchip sdk repo -> git
# Usages  :
# History : Please check the end of file.
###############################################################################

#git init;git add .gitignore;git commit -m "first commit, upload .gitignore"

git checkout -b kernel
git add -f kernel/; git commit -m "[stable-4.4-rk3399-linux](7379533) arm64: dts: rockchip: px30-evb-ddr3-v10: correction sw1/2 for rk809"
git checkout master

git checkout -b u-boot
git add -f u-boot/; git commit -m "[stable-4.4-rk3399-linux](d625acc) rockchip: board: fix cru phandle fixup failed"
git checkout master

git checkout -b rkbin
git add -f rkbin/; git commit -m "[stable-4.4-rk3399-linux](5e54e6c) rk1806: add initial version v1.00"
git checkout master

git checkout -b prebuilts
git add -f prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/
git add -f prebuilts/gcc/linux-x86/aarch64/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/
git commit -m "[master](dd4df7d) x86_64_arm https://releases.linaro.org/components/toolchain/binaries/6.3-2017.05/arm-linux-gnueabihf/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf.tar.xz
[master](f96f76d) x86_64_aarch64 https://releases.linaro.org/components/toolchain/binaries/6.3-2017.05/aarch64-linux-gnu/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu.tar.xz"
git checkout master

git checkout -b tools
git add -f tools/; git commit -m "[master](376143e) windows: add Rockchip_Develop_Tool v2.63 for Firmware.img upgrade"
git checkout master

git checkout -b device
git add -f device/rockchip/
git add -f build.sh burn.sh envsetup.sh Makefile mkfirmware.sh Readme repo.sh rkflash.sh rk-repo2git.sh temi-key
git commit -m "[master](39ea63e) echo location of br.log at the end of compilation and tail the log when failed"
git checkout master

git checkout -b docs
git add -f docs/; git commit -m "[master](7cf0615) Soc_public: update to v2.3.1 for rk3399"
git checkout master

git checkout -b ubuntu
git add -f ubuntu/; git commit -m "[None](None) upload offical ubuntu16.04 base rootfs"
git checkout master

git checkout -b debian
git add -f debian/; git commit -m "[master](c81ae7a) overlay-firmware: update npu_upgrade_pcie"
git checkout master

mkdir backup
git checkout -b buildroot
mv buildroot/dl/.git backup
git add -f buildroot/
mv backup/.git buildroot/dl
git commit -m "[stable-rk3399-v2.3.0-20191203](c81ae7a) package/rockchip: add linux-rga DEPENDENCIES for ffmpeg"
git checkout master

git checkout -b distro
mv distro/download/.git backup
git add -f distro/
mv backup/.git distro/download
git commit -m "[master](5d71d44) Merge changes I55da19f6,I07bc416c"
git checkout master
rmdir backup

git checkout -b yocto
git add -f yocto/poky/;
git add -f yocto/meta-clang/;
git add -f yocto/build/conf/;
git add -f yocto/meta-rockchip/;
git add -f yocto/meta-browser/;
git add -f yocto/meta-qt5/;
git add -f yocto/meta-rust/;
git add -f yocto/meta-openembedded/;
git add -f yocto/bitbake yocto/meta-poky yocto/oe-init-build-env yocto/scripts yocto/.templateconf; 
git commit -m "[None](mixed) upload rockchip yocto"
git checkout master

git checkout -b app
git add -f app/demo/; 
git add -f app/multivideoplayer/; 
git add -f app/qcamera/; 
git add -f app/qfm/; 
git add -f app/QLauncher/; 
git add -f app/qplayer/; 
git add -f app/qsetting/; 
git commit -m "[None](mixed) upload rockchip app"
git checkout master

git checkout -b external
git add -f external/gstreamer-rockchip/; 
git add -f external/wifiAutoSetup/; 
git add -f external/uvc_app/; 
git add -f external/rknpu/; 
git add -f external/LocalPlayer/; 
git add -f external/chromium/; 
git add -f external/rkssd/; 
git add -f external/tensorflow/; 
git add -f external/mpv/; 
git add -f external/security/bin/; 
git add -f external/security/rk_tee_user/; 
git add -f external/app/; 
git add -f external/minigui_desktop/; 
git add -f external/mpp/; 
git add -f external/bluez-alsa/; 
git add -f external/rockx/; 
git add -f external/rkscript/; 
git add -f external/avb/; 
git add -f external/rk_pcba_test/; 
git add -f external/broadcom_bsa/; 
git add -f external/libmali/; 
git add -f external/camera_engine_rkisp/; 
git add -f external/RKNPUTools/; 
git add -f external/bluetooth_bsa/; 
git add -f external/ble_wificonfig/; 
git add -f external/update_engine/; 
git add -f external/rknn-toolkit/; 
git add -f external/rkwifibt/; 
git add -f external/rtc_demo/; 
git add -f external/deviceio_release/; 
git add -f external/rk_hw_vad/; 
git add -f external/eq_drc_process/; 
git add -f external/wakeWordAgent/; 
git add -f external/recovery/; 
git add -f external/minigui/; 
git add -f external/audioservice/; 
git add -f external/rknn_demo/; 
git add -f external/libdrm/; 
git add -f external/linux-rga/; 
git add -f external/alsa-config/; 
git add -f external/rk_webui/; 
git add -f external/rkupdate/; 
git add -f external/rktoolkit/; 
git add -f external/softapDemo/; 
git add -f external/cae_vad/; 
git add -f external/rkmedia/; 
git add -f external/ipc_share_memory/; 
git add -f external/softapServer/; 
git add -f external/powermanager/; 
git commit -m "[None](mixed) upload rockchip external"
git checkout master

git checkout -b temi-master
# git merge kernel    --squash; git commit -m "Merge [kernel]() into temi-master"
# git merge u-boot    --squash; git commit -m "Merge [u-boot]() into temi-master"
# git merge rkbin     --squash; git commit -m "Merge [rkbin]() into temi-master"
# git merge prebuilts --squash; git commit -m "Merge [prebuilts]() into temi-master"
# git merge tools     --squash; git commit -m "Merge [tools]() into temi-master"
# git merge device    --squash; git commit -m "Merge [device]() into temi-master"
# git merge docs      --squash; git commit -m "Merge [docs]() into temi-master"
# git merge ubuntu    --squash; git commit -m "Merge [ubuntu]() into temi-master"

###############################################################################
# Author        Date        Version    Abstract
#------------------------------------------------------------------------------
# Kevin Oscar   2019-04-03  v1.0       Initial version create
###############################################################################
