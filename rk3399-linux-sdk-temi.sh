#!/bin/bash

git clone temi@172.16.6.6:/home/temi/workspace/rk3399-linux-sdk-temi && cd rk3399-linux-sdk-temi

cat > .git/config << EOF
[core]
	repositoryformatversion = 0
	filemode = true
	bare = false
	logallrefupdates = true
[credential]
	helper = store
[http]
	postBuffer = 1048576000
	sslverify = false

#add by kevin for support 'git push'
[receive]
	denyCurrentBranch = ignore

[lfs]
	url = "http://USER:PASS@172.16.6.6:9999/"
	#url = https://github.com/gfa99/rk3399_devenv.git
	#url = https://github.com/linshiyong525/rk3399_devenv.git
#[lfs "https://github.com/linshiyong525/rk3399_devenv.git/info/lfs"]
#[lfs "https://github.com/gfa99/rk3399_devenv.git/info/lfs"]
[lfs "http://USER:PASS@172.16.6.6:9999/"]
	access = basic
	locksverify = false

[remote "origin"]
	url = temi@172.16.6.6:/home/temi/workspace/rk3399-linux-sdk-temi
	#url = https://github.com/gfa99/rk3399_devenv.git
	#url = https://github.com/linshiyong525/rk3399_devenv.git
	fetch = +refs/heads/*:refs/remotes/origin/*
[branch "master"]
	remote = origin
	merge = refs/heads/master
EOF

#git fetch

git checkout -b yocto origin/yocto
git checkout -b ubuntu origin/ubuntu
git checkout -b u-boot origin/u-boot
git checkout -b tools origin/tools
git checkout -b rkbin origin/rkbin
git checkout -b prebuilts origin/prebuilts
git checkout -b kernel origin/kernel
git checkout -b git-lfs-cfg origin/git-lfs-cfg
git checkout -b external origin/external
git checkout -b docs origin/docs
git checkout -b distro origin/distro
git checkout -b device origin/device
git checkout -b debian origin/debian
git checkout -b buildroot origin/buildroot
git checkout -b app origin/app

git checkout -b TEMI_ubuntu origin/TEMI_ubuntu
git checkout -b TEMI_u-boot origin/TEMI_u-boot
git checkout -b TEMI_tools origin/TEMI_tools
git checkout -b TEMI_rkbin origin/TEMI_rkbin
git checkout -b TEMI_prebuilts origin/TEMI_prebuilts
git checkout -b TEMI_kernel origin/TEMI_kernel
git checkout -b TEMI_docs origin/TEMI_docs
git checkout -b TEMI_device origin/TEMI_device
git checkout -b TEMI_buildroot origin/TEMI_buildroot

git checkout -b temi-master origin/temi-master
git checkout -b temi-dev-env origin/temi-dev-env
git checkout -b TEMI_sync_rockchip origin/TEMI_sync_rockchip

git checkout -b temp-dev-env origin/temp-dev-env
git checkout -b temi-tmp-env origin/temi-tmp-env
