#!/bin/bash

IMG_NAME="update.img"

pause()
{
    echo "Press any key to quit:"
    read -n1 -s key
    exit 1
}

if [ -n "$1" ]; then
    IMG_NAME="$1"
fi

echo "start to unpack ${IMG_NAME} ..."

if [ ! -d "output" ]; then
	mkdir output
fi

if [ ! -f ${IMG_NAME} ]; then
	echo "Error:No found ${IMG_NAME}!"
	pause
fi

./rkImageMaker -unpack ${IMG_NAME} output || pause
./afptool -unpack output/firmware.img output || pause
rm -f output/firmware.img
rm -f output/boot.bin

echo "Unpacking ${IMG_NAME} OK."
echo "Press any key to quit:"
read -n1 -s key
exit 0