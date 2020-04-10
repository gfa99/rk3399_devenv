#!/bin/bash

# call this on 3399

input=/dev/video0
uvcvideo=

until [ $uvcvideo ]
do
    for dev in `ls /dev/video*`
    do
        v4l2-ctl -d $dev -D | grep rkisp1_mainpath > /dev/null
        if [ $? -eq 0 -a -z "$rkisp_main" ]; then rkisp_main=$dev; fi
        v4l2-ctl -d $dev -D | grep uvcvideo > /dev/null
        if [ $? -eq 0 -a -z "$uvcvideo" ]; then uvcvideo=$dev; fi
        v4l2-ctl -d $dev -D | grep cif > /dev/null
        if [ $? -eq 0 -a -z "$cifvideo" ]; then cifvideo=$dev; fi
    done
    if [ ! $1 ]; then break; fi
done
# uvc first, then rkisp
echo uvcvideo=$uvcvideo,rkisp_main=$rkisp_main,cifvideo=$cifvideo

if test $uvcvideo; then
    input=$uvcvideo
    sleep 1
else
    echo "no uvc camera found!"
    exit -1
fi
export SDL2_DISPLAY_PLANE_TYPE=OVERLAY
# PC: adb push external/rknpu/rknn/rknn_api/examples/rknn_ssd_demo/model/box_priors.txt /usr/bin/
# PC: adb push external/rknpu/rknn/rknn_api/examples/rknn_ssd_demo/model/coco_labels_list.txt /usr/bin/
rk_npu_uvc_host -i $input -w 1280 -h 720 -r 90
