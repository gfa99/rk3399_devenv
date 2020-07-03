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
echo "building pkgs: $PKGS"
if [ x$INIT == "xinit" ];then
	run $SCRIPTS_DIR/install.sh $RK_ARCH $SUITE "$PKGS" $INIT
else
	for p in $PKGS;do
		if [ -e $BUILD_DIR/$p/.timestamp ];then
			if [ -z `find $BUILD_DIR/$p -newer $BUILD_DIR/$p/.timestamp` ];then
				continue
			fi
		fi

        	if [ -x $PACKAGE_DIR/$p/make.sh ];then
			echo "building package $p"
			mkdir -p $BUILD_DIR/$p
                	run $PACKAGE_DIR/$p/make.sh
			touch $BUILD_DIR/$p/.timestamp
			echo "build $p done!!!"
		else
			run $SCRIPTS_DIR/install.sh $RK_ARCH $SUITE $p
        	fi
	done
fi

