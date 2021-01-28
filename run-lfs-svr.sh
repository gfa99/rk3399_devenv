#!/bin/bash

topdir=$(dirname "$0")
cd $topdir/.git/lfs-server
nohup bash ./bin/run-lfs-server.sh &

