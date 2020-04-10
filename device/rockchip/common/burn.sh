#!/bin/bash

# l: rk3399_loader_v1.22.115.bin, p: parameter.txt ... A: All
OPT_STR="lpubtmrkbfRODdAh"

IMG_PATH=rockdev
CMD_TOOL=tools/linux/Linux_Upgrade_Tool/Linux_Upgrade_Tool/upgrade_tool

function cmd_line_args_parse()
{
    while getopts ${OPT_STR} arg
    do
        case "$arg" in
            l) sudo ${CMD_TOOL} ul ${IMG_PATH}/MiniLoaderAll.bin ;;
            p) sudo ${CMD_TOOL} di -p ${IMG_PATH}/parameter.txt ;;
            u) sudo ${CMD_TOOL} di -u ${IMG_PATH}/uboot.img ;;
            t) sudo ${CMD_TOOL} di -t ${IMG_PATH}/trust.img ;;
            m) sudo ${CMD_TOOL} di -m ${IMG_PATH}/misc.img ;;

            r) sudo ${CMD_TOOL} di resource ${IMG_PATH}/resource.img ;;
            k) sudo ${CMD_TOOL} di -k ${IMG_PATH}/kernel.img ;;

            b) sudo ${CMD_TOOL} di -b ${IMG_PATH}/boot.img ;;
            R) sudo ${CMD_TOOL} di -r ${IMG_PATH}/recovery.img ;;
            f) sudo ${CMD_TOOL} di -rootfs ${IMG_PATH}/rootfs.img ;;
            O) sudo ${CMD_TOOL} di -oem ${IMG_PATH}/oem.img ;;
			D) sudo ${CMD_TOOL} di -userdata ${IMG_PATH}/userdata.img ;;
            d) sudo ${CMD_TOOL} rd ;;
            A)  
                # http://wiki.t-firefly.com/AIO-3399C/upgrade_firmware.html
                # 如果因 flash 问题导致升级时出错，可以尝试低级格式化、擦除 nand flash：
                # sudo upgrade_tool lf update.img # 低级格式化
                # sudo upgrade_tool ef update.img # 擦除flash 使用ef参数需要指定loader文件或者对应的update.img
                # sudo upgrade_tool uf update.img # 重新烧写

                # 烧写分区镜像：Ubuntu(MBR)、Android7.1、Android8.1,使用以下方式:
                # sudo ${CMD_TOOL} di -b /path/to/boot.img
                # sudo ${CMD_TOOL} di -k /path/to/kernel.img
                # sudo ${CMD_TOOL} di -s /path/to/system.img
                # sudo ${CMD_TOOL} di -r /path/to/recovery.img
                # sudo ${CMD_TOOL} di -m /path/to/misc.img
                # sudo ${CMD_TOOL} di resource /path/to/resource.img
                # sudo ${CMD_TOOL} di -p paramater   # 烧写 parameter
                # sudo ${CMD_TOOL} ul bootloader.bin # 烧写 bootloader

                # Ubuntu(GPT),使用以下方式
                sudo ${CMD_TOOL} ul ${IMG_PATH}/MiniLoaderAll.bin
                sudo ${CMD_TOOL} di -p ${IMG_PATH}/parameter.txt
                sudo ${CMD_TOOL} di -uboot ${IMG_PATH}/uboot.img
                sudo ${CMD_TOOL} di -trust ${IMG_PATH}/trust.img
                sudo ${CMD_TOOL} di -misc ${IMG_PATH}/misc.img
                sudo ${CMD_TOOL} di -b ${IMG_PATH}/boot.img
                sudo ${CMD_TOOL} di -recovery ${IMG_PATH}/recovery.img
                sudo ${CMD_TOOL} di -oem ${IMG_PATH}/oem.img
                sudo ${CMD_TOOL} di -rootfs ${IMG_PATH}/rootfs.img
                sudo ${CMD_TOOL} di -userdata ${IMG_PATH}/userdata.img
                ;;
            h|?)
                [ "$arg" != "h" ] && echo "Unkown arg, do nothing!"
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
                echo "           -A means burn All modules"
                echo "eg: ./burn.sh -ku means burn kernel.img and uboot.img"
                return 1
                ;;
        esac
    done

    return 0
}

function main()
{
    if [ $# -gt 0 ]; then
        cmd_line_args_parse $@
    else
        cmd_line_args_parse -A
    fi
}

main $@
