# Copyright (C) 2019, Fuzhou Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

DESCRIPTION = "A V4L2 plugin that wraps rockchip-mpp for the chromium's V4L2 VDA"

SECTION = "libs"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d749e86a105281d7a44c2328acebc4b0"

PV_append = "+git${SRCPV}"

inherit freeze-rev

SRCREV = "361b6b5b222b1832078387a4f2db073d7814712d"
SRC_URI = "git://github.com/JeffyCN/libv4l-rkmpp.git;branch=master"

S = "${WORKDIR}/git"

DEPENDS = "rockchip-mpp rockchip-librga libv4l"

inherit autotools pkgconfig

FILES_${PN} = "${libdir}/libv4l/plugins/*.so"
