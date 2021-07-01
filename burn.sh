#!/bin/bash

sudo ./tools/upgrade_tool ul output/rk3399_loader_v1.22.115.bin
sudo ./tools/upgrade_tool di -p output/parameter.txt
sudo ./tools/upgrade_tool di uboot output/uboot.img output/parameter.txt

sudo ./tools/upgrade_tool di -k output/kernel.img output/parameter.txt
sudo ./tools/upgrade_tool di resource output/resource.img output/parameter.txt
sudo ./tools/upgrade_tool rd

