#!/bin/bash

git_clone() {
#git clone file://127.0.0.1:/home/temi/workspace/rk3399-linux-sdk-temi && cd rk3399-linux-sdk-temi
git clone temi@172.16.6.6:/home/temi/workspace/rk3399-linux-sdk-temi && cd rk3399-linux-sdk-temi
#git clone git://172.16.6.6/temi-rk3399-linux-sdk.git && cd rk3399-linux-sdk-temi
}

git_config_modify() {
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

[filter "lfs"]
        required = true
        clean = git-lfs clean -- %f
        smudge = git-lfs smudge -- %f
        process = git-lfs filter-process
[lfs]
	url = "http://lfs_admin_user:lfs_admin_pass@172.16.6.6:9999/"
	#url = https://github.com/gfa99/rk3399_devenv.git
	#url = https://github.com/linshiyong525/rk3399_devenv.git
#[lfs "https://github.com/linshiyong525/rk3399_devenv.git/info/lfs"]
#[lfs "https://github.com/gfa99/rk3399_devenv.git/info/lfs"]
[lfs "http://lfs_admin_user:lfs_admin_pass@172.16.6.6:9999/"]
	access = basic
	locksverify = false

[remote "origin"]
	#url = file://127.0.0.1:/home/temi/workspace/rk3399-linux-sdk-temi
	url = temi@172.16.6.6:/home/temi/workspace/rk3399-linux-sdk-temi
	#url = git://172.16.6.6/temi-rk3399-linux-sdk.git
	#url = https://github.com/gfa99/rk3399_devenv.git
	#url = https://github.com/linshiyong525/rk3399_devenv.git
	fetch = +refs/heads/*:refs/remotes/origin/*
[branch "master"]
	remote = origin
	merge = refs/heads/master
EOF
}

git_checkout_all_branches() {
#git fetch

git checkout -b RK_prebuilts origin/RK_prebuilts
git checkout -b RK_kernel    origin/RK_kernel
git checkout -b RK_u-boot    origin/RK_u-boot
git checkout -b RK_ubuntu    origin/RK_ubuntu
git checkout -b RK_device    origin/RK_device
git checkout -b RK_rkbin     origin/RK_rkbin
git checkout -b RK_tools     origin/RK_tools
git checkout -b RK_docs      origin/RK_docs

git checkout -b RK_buildroot origin/RK_buildroot
git checkout -b RK_debian    origin/RK_debian
git checkout -b RK_yocto     origin/RK_yocto
git checkout -b RK_external  origin/RK_external
git checkout -b RK_distro    origin/RK_distro
git checkout -b RK_app       origin/RK_app

git checkout -b git-lfs-cfg  origin/git-lfs-cfg

git checkout -b TEMI_prebuilts origin/TEMI_prebuilts
git checkout -b TEMI_kernel    origin/TEMI_kernel
git checkout -b TEMI_u-boot    origin/TEMI_u-boot
git checkout -b TEMI_ubuntu    origin/TEMI_ubuntu
git checkout -b TEMI_device    origin/TEMI_device
git checkout -b TEMI_rkbin     origin/TEMI_rkbin
git checkout -b TEMI_tools     origin/TEMI_tools
git checkout -b TEMI_docs      origin/TEMI_docs

git checkout -b TEMI_buildroot origin/TEMI_buildroot
git checkout -b TEMI_sync_rockchip origin/TEMI_sync_rockchip
git checkout -b Sync_rockchip_sdk  origin/Sync_rockchip_sdk

git checkout -b temi-master origin/temi-master
git checkout -b temi-dev-env origin/temi-dev-env
git checkout -b TEMI_x3399_sdk origin/TEMI_x3399_sdk

git checkout -b temi-tmp-env origin/temi-tmp-env
}

git_merge_rockchip_sdk() {
git merge --no-ff RK_prebuilts RK_kernel RK_u-boot RK_device RK_rkbin RK_tools RK_docs

git merge --no-ff RK_app RK_yocto RK_debian RK_distro RK_external RK_buildroot
}

main () {
git_clone
[ $? -eq 0 ] && git_config_modify
[ $? -eq 0 ] && git_checkout_all_branches
[ $? -eq 0 ] && cp ../clone-temi-rk3399-linux-sdk.sh .
}

main $@
