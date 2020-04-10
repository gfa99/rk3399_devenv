#!/bin/bash

set -e
ARCH=$1
SUITE=$2
PKGS=$3
INIT=$4

log() {
    local format="$1"
    shift
    printf -- "$format\n" "$@" >&2
}

run() {
    log "I: Running command: %s" "$*"
    "$@"
}

OS=`$SCRIPTS_DIR/get_distro.sh $SUITE`
pkgs=""
echo "installing package $PKGS"
for p in $PKGS;do
	if [ -e $BUILD_DIR/$p/.timestamp ];then
		if [ -z `find $BUILD_DIR/$p -newer $BUILD_DIR/$p/.timestamp` ];then
			echo "$p installed before, skiped"
			continue
		fi
	fi
	pkgs="$pkgs $p"
done

if [ -z "$pkgs" ];then
	echo "all packages installed, skip"
	exit 0
fi

if [ x$INIT == "xinit" ];then
       	if [ $OS == debian ];then
		run $SCRIPTS_DIR/install_debian_pkg.sh $RK_ARCH $SUITE "$pkgs" $INIT
	elif [ $OS == ubuntu ];then
		run $SCRIPTS_DIR/install_ubuntu_pkg.sh $RK_ARCH $SUITE "$pkgs" $INIT
	fi
else
	if [ $OS == debian ];then
		run $SCRIPTS_DIR/install_debian_pkg.sh $RK_ARCH $SUITE "$pkgs"
	elif [ $OS == ubuntu ];then
		run $SCRIPTS_DIR/install_ubuntu_pkg.sh $RK_ARCH $SUITE "$pkgs"
	fi
fi

for p in $pkgs;do
	mkdir -p $BUILD_DIR/$p
	touch $BUILD_DIR/$p/.timestamp
done

echo "install $pkgs done!!!"
