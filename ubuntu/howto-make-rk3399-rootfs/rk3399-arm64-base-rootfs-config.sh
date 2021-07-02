# Version 0.1 (2021-06-01, base on ubuntu-20.04-base)
#===================================================================================================
# 1. Make rk3399(arm64) image (no gui) based on ubuntu-base-16.04.6
# 2. install some necessary software such as: 
#    systemd rsyslog sudo nano android-tools-adb bash-completion
#    net-tools network-manager openssh-server rsync
# 3. configure user 'temi' and set hostname to 'temi'
# 4. configure user 'ninetripod' for robox
# 5. configure login via serial-port
# 6. modify /etc/rc.local to disable hibernate, adjust hdmi output and so on
# 7. fix two displayers' primary and secondary order (ref to /etc/profile)
#---------------------------------------------------------------------------------------------------
apt update
apt upgrade

apt install systemd rsyslog sudo nano
apt install net-tools network-manager
apt install openssh-server rsync
apt install bash-completion
apt install android-tools-adb
  
useradd temi -s /bin/bash -mr -G adm,sudo
passwd temi << EOF
temi
temi
EOF

adduser ninetripod
groups=(adm sudo)
for agroup in ${groups[@]}; do
  adduser ninetripod $agroup
done
passwd ninetripod << EOF
Algo!@Robo#$01%10^18
Algo!@Robo#$01%10^18
EOF
echo "TEMI" > /etc/hostname
# 修正[unable to resolve host](https://blog.csdn.net/ichuzhen/article/details/8241847)
echo "127.0.0.1 localhost TEMI" >> /etc/hosts

# 修正[Ubuntu 16.04 制作rootfs无法启动，串口ttyFIQ不正常](https://dev.t-firefly.com/thread-13162-1-13.html)
sed -i 's/^BindsTo=dev-%i.device/BindsTo=dev-%i/g'                              /lib/systemd/system/serial-getty@.service
sed -i '/^BindsTo=dev-%i/i\# https://dev.t-firefly.com\/thread-13162-1-13.html' /lib/systemd/system/serial-getty@.service

sudo cat > /etc/rc.local << EOF
#!/bin/sh -e
#
# rc.local
#
# This script is executed at the end of each multiuser runlevel.
# Make sure that the script will "exit 0" on success or any other
# value on error.
#
# In order to enable or disable this script just change the execution
# bits.
#
# By default this script does nothing.

# Only run once at first power up, to improve root file system
bash /home/temi/.run_at_first_powerup.sh && sed -i 's/^bash.*run_at_first_powerup.sh/# &/g' /etc/rc.local

# Disable Hibernate
systemctl mask sleep.target suspend.target hibernate.target hybrid-sleep.target

echo "/etc/rc.local autostart test: `date`" > /tmp/rc.local.log

exit 0
EOF

sudo cat >> /etc/profile << EOF
# Add by kevin, for show system-menu-bar
xrandr --output eDP-1 --right-of HDMI-1 --auto  &>/dev/null
#xrandr --output HDMI-1 --same-as DSI-1 --auto   &>/dev/null
xrandr --output HDMI-1 --auto --primary         &>/dev/null
EOF

# Version 0.2 (2021-06-02)
#===================================================================================================
# 1. install desktop environment 'lxde' and its's display managers 'lightdm'
# 2. configure auto-login gui and text-cmd-line console (ttyFIQ0) by ninetripod
# 3. configure user 'ninetripod' no need input password when use 'sudo'
# 4. Add extra swap partition and fill rootfs into the entire partition
#---------------------------------------------------------------------------------------------------
apt install lightdm lxde

# 配置以 ninetripod 自动登录图形界面
# https://www.helplib.cn/beryl/enable-or-disable-automatic-login-in-ubuntu-20-04
sudo cat > /etc/lightdm/lightdm.conf.d/autologin.conf << EOF
[SeatDefaults]
autologin-user=ninetripod
autologin-user-timeout=0
EOF

# 配置以 ninetripod 自动登录串口
# https://blog.csdn.net/a617996505/article/details/88423794
# 执行 nano /lib/systemd/system/serial-getty@.service
# 将ExecStart所在行修改为如下内容
# ExecStart=-/sbin/agetty -a ninetripod --keep-baud 115200,38400,9600 %I $TERM
sudo sed -i 's/^ExecStart=.*/ExecStart=-\/sbin\/agetty -a ninetripod --keep-baud 115200,38400,9600 %I $TERM/g' /lib/systemd/system/serial-getty@.service

# 配置 ninetripod 用户使用 sudo 时无需输入密码
# 执行 sudo visuders
# 在打开的文件中增加如下配置
# ninetripod ALL=(ALL) NOPASSWD:ALL
sudo echo "ninetripod ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# 配置交换分区，并将根文件系统扩展到整个分区
cat > /home/temi/.run_at_first_powerup.sh << EOF
#!/bin/env bash

# Let the rootfs fill the entire partition
function resize_rootfs_partition() {
	local partation=`grep -o "root=/dev/mmcblk1p1[0-9]" /proc/cmdline | cut -d "=" -f2`
	test -b $partation && resize2fs $partation
}

# Add extra file used to swap partition
function add_swap_partition() {
	if [ ! -s '/extra_swapfile' ]; then
	  echo "Creating extra_swapfile."
	  dd if=/dev/zero of=/extra_swapfile bs=1M count=2048
	  mkswap /extra_swapfile
	  swapon /extra_swapfile
	  if !(grep -Fxq "extra_swapfile" /etc/fstab); then
	    echo "/extra_swapfile none swap sw 0 0" >> /etc/fstab
	  fi
	fi
}

function main() {
	[[ $EUID -ne 0 ]] && echo "This script must be run as root." && exit 1

	resize_rootfs_partition
	
	add_swap_partition
}

main $@
EOF
