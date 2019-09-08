#!/bin/sh


sudo ./upgrade_tool ul rk3399_loader_v1.22.115.bin
sudo ./upgrade_tool di -p parameter.txt
sudo ./upgrade_tool di uboot uboot.img parameter.txt
sudo ./upgrade_tool di trust trust.img parameter.txt
sudo ./upgrade_tool di -m misc_zero.img parameter.txt
sudo ./upgrade_tool di resource resource.img parameter.txt
sudo ./upgrade_tool di -k kernel.img parameter.txt
sudo ./upgrade_tool di boot boot.img parameter.txt
 

#sudo ./upgrade_tool di userdata ubuntu-rootfs.img parameter.txt
sudo ./upgrade_tool rd
