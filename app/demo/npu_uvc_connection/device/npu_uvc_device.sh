#!/bin/bash
# call this file on 1808

for dev in `ls /dev/video*`
do
    v4l2-ctl -d $dev -D | grep rkisp1_mainpath > /dev/null
    if [ $? -eq 0 -a -z "$rkisp_main" ]; then rkisp_main=$dev; fi
    v4l2-ctl -d $dev -D | grep uvcvideo > /dev/null
    if [ $? -eq 0 -a -z "$uvcvideo" ]; then uvcvideo=$dev; fi
    v4l2-ctl -d $dev -D | grep cif > /dev/null
    if [ $? -eq 0 -a -z "$cifvideo" ]; then cifvideo=$dev; fi
done
# uvc first, then rkisp
echo uvcvideo=$uvcvideo,rkisp_main=$rkisp_main,cifvideo=$cifvideo

need_3a=0
if test $uvcvideo; then
    input=$uvcvideo
    input_format=image:jpeg
elif test $rkisp_main; then
    input=$rkisp_main
    input_format=image:nv12
    need_3a=1
    io -4 -w 0xfe8a0008 0xffff0202
elif test $cifvideo; then
    input=$cifvideo
    input_format=image:nv16
else
    echo find non valid camera via v4l2-ctl
fi
#input_format=image:yuv420p
uvc_MJPEG.sh 1280 720
# PC: adb push external/rknpu/rknn/rknn_api/examples/rknn_ssd_demo/model/ssd_inception_v2.rknn /userdata/
# model_file=/userdata/ssd_inception_v2.rknn
# if [ ! -f $model_file ]; then
#     echo "miss $model_file"
#     exit -1
# fi
# rk_npu_uvc_device -i $input -c $need_3a -f $input_format -w 1280 -h 720 -r 0 \
#         -m /userdata/ssd_inception_v2.rknn -n rknn_ssd:300x300

# gdb --args
# rockx_face_gender_age:300x300 / rockx_face_detect:300x300
rk_npu_uvc_device -i $input -c $need_3a -f $input_format -w 1280 -h 720 -r 1 -n rockx_face_gender_age:300x300
