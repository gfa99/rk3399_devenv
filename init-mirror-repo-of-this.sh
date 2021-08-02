#!/bin/bash

CURR_DIR=`pwd`
REPO_DIR=$(date +%Y%m%d%H%M%S)

echo "Please input the path of the mirror repo(for example:..):"
read git_repo_path

if [ -d ${git_repo_path} ]; then
	git_repo_path=$(realpath $git_repo_path)/${REPO_DIR}
	mkdir -p "$git_repo_path"
else
	echo "Error: no such path!"
	exit 1
fi

cd ${git_repo_path}
git init --bare temi-rk3399-linux-sdk.git
ln -sf temi-rk3399-linux-sdk.git .git
cp -rpf ${CURR_DIR}/lfs-server     temi-rk3399-linux-sdk.git/
cp -rpf ${CURR_DIR}/run-lfs-svr.sh ./ && sh ./run-lfs-svr.sh
cd ${CURR_DIR}

git remote set-url origin file://${git_repo_path}/temi-rk3399-linux-sdk.git
git push origin master
git push origin --mirror
