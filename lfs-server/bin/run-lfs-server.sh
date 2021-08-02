#!/bin/bash

set -eu
set -o pipefail

if [[ $# -ge 1 && -d "$1/.git" ]]; then
  git_repo_path=$1
else
  git_repo_path=`pwd`
fi

#server_home=${git_repo_path}/temi-rk3399-linux-sdk.git/lfs-server
server_home=${git_repo_path}/.git/lfs-server
server_addr="127.0.0.1"
server_port="9999"
server_user="lfs_admin_user"
server_pass="lfs_admin_pass"

LFS_LISTEN="tcp://:$server_port"
LFS_HOST="$server_addr:$server_port"
LFS_CONTENTPATH="${server_home}/data"
LFS_ADMINUSER="$server_user"
LFS_ADMINPASS="$server_pass"
LFS_SCHEME="http"

export LFS_LISTEN LFS_HOST LFS_CONTENTPATH LFS_ADMINUSER LFS_ADMINPASS LFS_SCHEME

cd ${server_home} && ./bin/lfs-test-server
