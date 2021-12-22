#!/bin/bash


function mnt() {
    if [ $(ls $2/proc | wc -l) -eq 0 ]; then
        echo "MOUNTING"
        sudo mount -t proc  /proc    ${2}/proc
        sudo mount -t sysfs /sys     ${2}/sys
        sudo mount -o bind  /dev     ${2}/dev
        sudo mount -o bind  /dev/pts ${2}/dev/pts
    else
        echo "Has Mounted!"
    fi
    sudo chroot ${2}
}

function umnt() {
    echo "UNMOUNTING"
    sudo umount ${2}/proc
    sudo umount ${2}/sys
    sudo umount ${2}/dev/pts
    sudo umount ${2}/dev

}

function mkrfs() {
    if [ $# -lt 1 ]; then
        echo "Error: No target directory"
        return 1
    elif [ ! -d $1 ]; then
        echo "Error: No such directory"
        return 1
    fi

    ROOTFS_DIR=$(date +%Y%m%d%H%M%S)
    ROOTFS_IMG=${ROOTFS_DIR}.img
    ROOTFS_TMP=$1
    TARGET_CNT=`sudo du -sh -BM $ROOTFS_TMP | awk '{print $1}' | sed 's/M//'`
    TARGET_CNT=$((TARGET_CNT+3399))
    echo "TARGET_CNT: $TARGET_CNT"

    mkdir        $ROOTFS_DIR
    dd if=/dev/zero of=$ROOTFS_IMG bs=1M count=$TARGET_CNT
    mkfs.ext4    $ROOTFS_IMG
    sudo mount   $ROOTFS_IMG   $ROOTFS_DIR
    sudo cp -rfp $ROOTFS_TMP/* $ROOTFS_DIR
    sudo umount  $ROOTFS_DIR
    e2fsck -p -f $ROOTFS_IMG
    resize2fs -M $ROOTFS_IMG
    rmdir        $ROOTFS_DIR
}

function main() {
    if [ -d $1 ]; then
        mkrfs $1
    elif [ "$1" == "-m" ] && [ -n "$2" ]; then # for local same arch rootfs
        if [ ! -d "$2" ]; then
            echo "Please ready for the base rootfs directory! Refer to the following:"
            echo "mkdir $2 && sudo tar ubuntu-base-16.04.6-base-arm64.tar.gz -xpf -C $2"
            exit 1
        fi
        
        type qemu-aarch64-static >/dev/null 2>&1

        if [ $? -ne 0 ]; then
            echo "qemu-user-static not installed. if you want to install?(y/n)"
            read -t 5 ans
            [ "$ans" = "y" ] && sudo apt install qemu-user-static
        fi

        if [ ! -s "$2/usr/bin/qemu-aarch64-static" ]; then
            sudo cp -b /etc/resolv.conf $2/etc/resolv.conf
            sudo cp /usr/bin/qemu-aarch64-static $2/usr/bin/
        fi

        mnt $1 $2
    elif [ "$1" == "-M" ] && [ -n "$2" ]; then # for other different arch rootfs
        mnt $1 $2
    elif [ "$1" == "-u" ] && [ -n "$2" ]; then
        umnt $1 $2
    else
        echo ""
        echo "Either 1'st, 2'nd or both parameters were missing"
        echo ""
        echo "1'st parameter can be one of these: -m(mount) OR -u(umount)"
        echo "2'nd parameter is the full path of rootfs directory(with trailing '/')"
        echo ""
        echo "For example: ch-mount -m /media/sdcard/"
        echo ""
        echo 1st parameter : ${1}
        echo 2nd parameter : ${2}
    fi
}

main $@
