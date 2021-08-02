#!/bin/bash

topdir=$(dirname $(realpath "$0"))
cd $topdir/.git/lfs-server
nohup bash ./bin/run-lfs-server.sh $topdir &
