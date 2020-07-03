1、buildroot配置开启（menuconfig或者defconfig里）：
+BR2_PACKAGE_FLAC=y
+BR2_PACKAGE_LIBERATION=y
+BR2_PACKAGE_LIBNSS=y
+BR2_PACKAGE_WEBP=y
+BR2_PACKAGE_WEBP_DEMUX=y
+BR2_PACKAGE_WEBP_MUX=y
+BR2_PACKAGE_LIBXSLT=y
+BR2_PACKAGE_LIBV4L_RKMPP=y

2、设置XDG_RUNTIME_DIR（必须和weston一致），比如：
export XDG_RUNTIME_DIR=/tmp/.xdg

3、替换mali库为特殊版本（https://github.com/JeffyCN/misc/tree/master/libmali-chromium），
比如：
adb push libmali.so_rk3326_wayland_gbm_arm64_rxp0_nocl /usr/lib/libmali.so
adb push libmali.so_rk3399_wayland_gbm_arm64_r14p0 /usr/lib/libmali.so

4、创建虚拟解码设备
touch /dev/video-dec0

5、启动chromium：
chromium --no-sandbox --gpu-sandbox-start-early --ozone-platform=wayland --ignore-gpu-blacklist --enable-wayland-ime
