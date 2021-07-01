#!/bin/bash

# l: rk3399_loader_v1.22.115.bin, p: parameter.txt ... A: All
OPT_STR="lpubtmrkbfdAh"

function cmd_line_args_parse()
{
    if [ ! -x ./upgrade_tool ]; then
        cp ../tools/upgrade_tool .
    fi

    while getopts ${OPT_STR} arg
    do
        case "$arg" in
            l)  sudo ./upgrade_tool ul rk3399_loader_v1.22.115.bin ;;
            p)  sudo ./upgrade_tool di -p parameter.txt ;;
            u)  sudo ./upgrade_tool di uboot uboot.img ;;
            t)  sudo ./upgrade_tool di trust trust.img ;;
            m)  sudo ./upgrade_tool di misc misc_zero.img ;;
            r)  sudo ./upgrade_tool di resource resource.img ;;
            k)  sudo ./upgrade_tool di kernel kernel.img ;;
            b)  sudo ./upgrade_tool di boot boot.img ;;
            f)  sudo ./upgrade_tool di userdata ubuntu-rootfs.img ;;
            d)  sudo ./upgrade_tool rd ;;
            A)
                sudo ./upgrade_tool ul rk3399_loader_v1.22.115.bin
                sudo ./upgrade_tool di -p parameter.txt
                sudo ./upgrade_tool di uboot uboot.img parameter.txt
                sudo ./upgrade_tool di trust trust.img parameter.txt
                sudo ./upgrade_tool di -m misc_zero.img parameter.txt
                sudo ./upgrade_tool di resource resource.img parameter.txt
                sudo ./upgrade_tool di -k kernel.img parameter.txt
                sudo ./upgrade_tool di boot boot.img parameter.txt
                 
                sudo ./upgrade_tool di userdata ubuntu-rootfs.img parameter.txt
                sudo ./upgrade_tool rd
                ;;
            h|?)
                [ "$arg" != "h" ] && echo "Unkown arg, do nothing!"
                echo 
                echo "Usage: ./burn.sh [OPTION]"
                echo "[OPTION]: [-l] [-p] [-u] [-t] [-m] [-r] [-k] [-b] [-f] [-d]"
                echo "           -l means burn rk3399_loader_v1.22.115.bin"
                echo "           -p means burn parameter.txt"
                echo "           -u means burn uboot.img"
                echo "           -t means burn trust.img"
                echo "           -m means burn misc_zero.img"
                echo "           -r means burn resource.img"
                echo "           -b means burn boot.img"
                echo "           -f means burn rootfs.img"
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
    cd output
    if [ $# -gt 0 ]; then
        cmd_line_args_parse $@
    else
        cmd_line_args_parse -A
    fi
    cd -
}

main $@