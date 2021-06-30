#!/bin/bash
###############################################################################
# Author  : Kevin Oscar<zrzxlfe@sina.com>
# Date    : 2019-12-22
# Version : v0.1
# Abstract: Touchpad of laptop on or off under ubuntu.
# Usages  :
# History : Please check the end of file.
###############################################################################

# l: MiniLoaderAll.bin, p: parameter.txt ... A: All
OPT_STR="lpubtmrkbfRODdEUAh"

NEW_PATH=rockdev
NEW_TOOL=tools/linux/Linux_Upgrade_Tool/Linux_Upgrade_Tool/upgrade_tool

OLD_PATH=rockdev
OLD_TOOL=tools/linux/Linux_Upgrade_Tool/Linux_Upgrade_Tool/upgrade_tool_v1.24
RFS_TYPE=ubuntu

function fw_burn_usage()
{
    echo 
    echo "Usage: ./burn.sh [OPTION]"
    echo "[OPTION]: [-l] [-p] [-u] [-t] [-m] [-r] [-k] [-b] [-f] [-d]"
    echo "           -l means burn MiniLoaderAll.bin"
    echo "           -p means burn parameter.txt"
    echo "           -u means burn uboot.img"
    echo "           -t means burn trust.img"
    echo "           -m means burn misc.img"
    echo "           -r means burn resource.img"
    echo "           -k means burn kernel.img"
    echo "           -b means burn boot.img (both of kernel resource)"
    echo "           -f means burn rootfs.img"
    echo "           -R means burn recovery.img"
    echo "           -O means burn oem.img"
    echo "           -D means burn userdata.img"
    echo "           -E means earse flash"
    echo "           -U means upgrade fireware package"
    echo "           -A means burn All modules"
    echo "eg: ./burn.sh -ku means burn kernel.img and uboot.img"
}

function earse_flash()
{
    local loader=$1

    sudo ${OLD_TOOL} lf ${loader} # 低级格式化
    sudo ${OLD_TOOL} ef ${loader} # 擦除flash
}

function old_mbr_fw_burn()
{
    while getopts ${OPT_STR} arg
    do
        case "$arg" in
            l) sudo ${OLD_TOOL} ul ${OLD_PATH}/rk3399_loader_v1.22.115.bin ;;
            p) sudo ${OLD_TOOL} di -p ${OLD_PATH}/parameter.txt ;;
            u) sudo ${OLD_TOOL} di uboot ${OLD_PATH}/uboot.img ;;
            t) sudo ${OLD_TOOL} di trust ${OLD_PATH}/trust.img ;;
            k) sudo ${OLD_TOOL} di kernel ${OLD_PATH}/kernel.img ;;
            r) sudo ${OLD_TOOL} di resource ${OLD_PATH}/resource.img ;;
            m) sudo ${OLD_TOOL} di misc ${OLD_PATH}/misc.img ;;
            b) sudo ${OLD_TOOL} di boot ${OLD_PATH}/boot.img ;;
            f) sudo ${OLD_TOOL} di userdata ${OLD_PATH}/${RFS_TYPE}.img ;;
            d) sudo ${OLD_TOOL} rd ;;
            E) earse_flash ${OLD_PATH}/rk3399_loader_v1.22.115.bin ;;
            U) echo "do nothing ..." ;;
            A) 
               sudo ${OLD_TOOL} ul rk3399_loader_v1.22.115.bin
               sudo ${OLD_TOOL} di -p parameter.txt
               sudo ${OLD_TOOL} di ${OLD_PATH}/uboot.img ${OLD_PATH}/parameter.txt
               sudo ${OLD_TOOL} di trust ${OLD_PATH}/trust.img ${OLD_PATH}/parameter.txt 
               sudo ${OLD_TOOL} di kernel ${OLD_PATH}/kernel.img ${OLD_PATH}/parameter.txt
               sudo ${OLD_TOOL} di resource ${OLD_PATH}/resource.img ${OLD_PATH}/parameter.txt
               sudo ${OLD_TOOL} di misc ${OLD_PATH}/misc.img ${OLD_PATH}/parameter.txt
               sudo ${OLD_TOOL} di boot ${OLD_PATH}/boot.img ${OLD_PATH}/parameter.txt

               sudo ${OLD_TOOL} di userdata ${OLD_PATH}/${RFS_TYPE}.img ${OLD_PATH}/parameter.txt
                ;;
            h|*|?)
               [ "$arg" != "h" ] && echo "Unkown arg, do nothing!"
               fw_burn_usage && return 1
                ;;
        esac
    done

    return 0
}

function new_gpt_fw_burn()
{
    while getopts ${OPT_STR} arg
    do
        case "$arg" in
            l) sudo ${NEW_TOOL} ul ${NEW_PATH}/MiniLoaderAll.bin ;;
            p) sudo ${NEW_TOOL} di -p ${NEW_PATH}/parameter.txt ;;
            u) sudo ${NEW_TOOL} di -u ${NEW_PATH}/uboot.img ;;
            t) sudo ${NEW_TOOL} di -t ${NEW_PATH}/trust.img ;;
            m) sudo ${NEW_TOOL} di -m ${NEW_PATH}/misc.img ;;

            r) sudo ${NEW_TOOL} di -resource ${NEW_PATH}/resource.img ;;
            k) sudo ${NEW_TOOL} di -k ${NEW_PATH}/kernel.img ;;

            b) sudo ${NEW_TOOL} di -b ${NEW_PATH}/boot.img ;;
            R) sudo ${NEW_TOOL} di -r ${NEW_PATH}/recovery.img ;;
            s) sudo ${NEW_TOOL} di -s ${NEW_PATH}/system.img ;;
            O) sudo ${NEW_TOOL} di -oem ${NEW_PATH}/oem.img ;;
            f) sudo ${NEW_TOOL} di -rootfs ${NEW_PATH}/${RFS_TYPE}.img ;;
            D) sudo ${NEW_TOOL} di -userdata ${NEW_PATH}/userdata.img ;;
            E) earse_flash ${NEW_PATH}/MiniLoaderAll.bin ;;
            U) sudo ${NEW_TOOL} uf ${NEW_PATH}/AIO-3399C-UBUNTU16.04-GPT-20190820-1710.img ;;
            d) sudo ${NEW_TOOL} rd ;;
            A) 
                # http://wiki.t-firefly.com/AIO-3399C/upgrade_firmware.html
                # 如果因 flash 问题导致升级时出错，可以尝试低级格式化、擦除 nand flash：
                # sudo upgrade_tool lf update.img # 低级格式化
                # sudo upgrade_tool ef update.img # 擦除flash 使用ef参数需要指定loader文件或者对应的update.img
                # sudo upgrade_tool uf update.img # 重新烧写

                # 烧写分区镜像：Ubuntu(MBR)、Android7.1、Android8.1,使用以下方式:
                # sudo ${NEW_TOOL} di -b /path/to/boot.img
                # sudo ${NEW_TOOL} di -k /path/to/kernel.img
                # sudo ${NEW_TOOL} di -s /path/to/system.img
                # sudo ${NEW_TOOL} di -r /path/to/recovery.img
                # sudo ${NEW_TOOL} di -m /path/to/misc.img
                # sudo ${NEW_TOOL} di resource /path/to/resource.img
                # sudo ${NEW_TOOL} di -p paramater   # 烧写 parameter
                # sudo ${NEW_TOOL} ul bootloader.bin # 烧写 bootloader

                # Ubuntu(GPT),使用以下方式
                sudo ${NEW_TOOL} ul ${NEW_PATH}/MiniLoaderAll.bin
                sudo ${NEW_TOOL} di -p ${NEW_PATH}/parameter.txt
                sudo ${NEW_TOOL} di -uboot ${NEW_PATH}/uboot.img
                sudo ${NEW_TOOL} di -trust ${NEW_PATH}/trust.img
                sudo ${NEW_TOOL} di -misc ${NEW_PATH}/misc.img
                sudo ${NEW_TOOL} di -b ${NEW_PATH}/boot.img
                sudo ${NEW_TOOL} di -recovery ${NEW_PATH}/recovery.img
                # sudo ${NEW_TOOL} di -system ${NEW_PATH}/system.img
                # sudo ${NEW_TOOL} di -oem ${NEW_PATH}/oem.img
                # sudo ${NEW_TOOL} di -userdata ${NEW_PATH}/userdata.img
                sudo ${NEW_TOOL} di -rootfs ${NEW_PATH}/${RFS_TYPE}.img
                ;;
            h|?)
                [ "$arg" != "h" ] && echo "Unkown arg, do nothing!"
                fw_burn_usage && return 1
                ;;
        esac
    done

    return 0
}

function main()
{
    if [ "$1" = "old" ]; then
        shift
        if [ $# -gt 0 ]; then
            old_mbr_fw_burn $@
        else
            old_mbr_fw_burn -A
        fi
    else
        if [ $# -gt 0 ]; then
            new_gpt_fw_burn $@
        else
            new_gpt_fw_burn -A
        fi
    fi
}

main $@

###############################################################################
# Author        Date        Version    Abstract
#------------------------------------------------------------------------------
# Kevin Oscar   2019-12-22  v0.1       Initial version create
# Kevin Oscar   2020-04-13  v1.0       Initial version create
#==============================================================================
# 采用Rockchip主线代码和Firefly AIO-3399C的dts、config相结合来适配我们的板(基于九鼎X33399)
# 其中，Rockchip: kernel(7379533) u-boot(d625acc)
#       Firelfy: rk3399-firefly-aioc.dts(866527f), firefly_linux_defconfig(21cea54)
# 烧录方法：
#     1. 对于MBR分区的旧板，需要使用upgrade_tool_v1.24先擦除
#        1）sudo ./upgrade_tool lf MiniLoaderAll.bin
#        2）sudo ./upgrade_tool ef MiniLoaderAll.bin
#     2. 升级新的固件（GPT分区）
#        3）依次执行如下命令，烧录各组件(upgrade_tool >= v1.34)
#           sudo ./upgrade_tool ul ${IMG_PATH}/MiniLoaderAll.bin
#           sudo ./upgrade_tool di -p ${IMG_PATH}/parameter.txt
#           sudo ./upgrade_tool di -u ${IMG_PATH}/uboot.img
#           sudo ./upgrade_tool di -t ${IMG_PATH}/trust.img
#           sudo ./upgrade_tool di -b ${IMG_PATH}/boot.img
#           sudo ./upgrade_tool di -rootfs ${IMG_PATH}/rootfs.img
#     3. 还原旧的固件（MBR分区），全过程中均使用upgrade_tool_v1.24
#        4）首先执行步骤1中的擦除操作
#        5）之后依次执行如下命令：
#           sudo ./upgrade_tool ul rk3399_loader_v1.22.115.bin
#           sudo ./upgrade_tool di -p parameter.txt
#           sudo ./upgrade_tool di uboot uboot.img parameter.txt
#           sudo ./upgrade_tool di trust trust.img parameter.txt
#           sudo ./upgrade_tool di resource resource.img parameter.txt
#           sudo ./upgrade_tool di kernel kernel.img parameter.txt
#           sudo ./upgrade_tool di boot boot.img parameter.txt
#           sudo ./upgrade_tool di misc misc_zero.img parameter.txt
#           sudo ./upgrade_tool di userdata ubuntu-rootfs.img parameter.txt
#    参考：
#     1）烧写须知(重要)(http://wiki.t-firefly.com/AIO-3399C/upgrade_table.html)
#     2）升级固件(http://wiki.t-firefly.com/AIO-3399C/upgrade_firmware.html)
#     3) Firefly rootfs(https://pan.baidu.com/s/1DuCzTGARDi7APxyKs9Nl1A#list/path=%2F)
###############################################################################
