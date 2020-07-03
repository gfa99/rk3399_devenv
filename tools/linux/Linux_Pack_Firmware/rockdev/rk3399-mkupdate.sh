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

echo "start to make ${IMG_NAME} ..."

if [ ! -f "Image/parameter" -a ! -f "Image/parameter.txt" ]; then
	echo "Error:No found parameter!"
	exit 1
fi

if [ ! -f "package-file" ]; then
	echo "Error:No found package-file!"
	exit 1
fi

./afptool -pack ./ Image/${IMG_NAME} || pause
./rkImageMaker -RK330C Image/MiniLoaderAll.bin Image/${IMG_NAME} ${IMG_NAME} -os_type:androidos || pause

# update.img is new format, Image/update.img is old format, so delete older format
rm -f Image/${IMG_NAME}

echo "Making ${IMG_NAME} OK."
echo "Press any key to quit:"
read -n1 -s key
exit $?
