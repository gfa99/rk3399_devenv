# Copyright (C) 2020, Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

DESCRIPTION = "Rockchip ALSA config files"
SECTION = "multimedia"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://NOTICE;md5=9645f39e9db895a4aa6e02cb57294595"

PV_append = "+git${SRCPV}"

inherit freeze-rev

SRC_URI = " \
	git://github.com/JeffyCN/mirrors.git;branch=alsa-config; \
"
SRCREV = "2f464eefd27973142bda49c952172695a161fbc3"
S = "${WORKDIR}/git"

inherit meson

FILES_${PN} = "*"
