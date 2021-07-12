SUMMARY = "Modern password hashing for your software and your servers."
DESCRIPTION = "Good password hashing for your software and your servers"
HOMEPAGE = "https://github.com/pyca/bcrypt/"
SECTION = "devel/python"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8f7bb094c7232b058c7e9f2e431f389c"

SRC_URI[md5sum] = "5d6f93b575ce52470af37a8e7dce76fe"
SRC_URI[sha256sum] = "0b0069c752ec14172c5f78208f1863d7ad6755a6fae6fe76ec2c80d13be41e42"

DEPENDS += "${PYTHON_PN}-cffi-native"

inherit pypi setuptools

RDEPENDS_${PN}_class-target += "\
    ${PYTHON_PN}-cffi \
    ${PYTHON_PN}-ctypes \
    ${PYTHON_PN}-shell \
    ${PYTHON_PN}-six \
    ${PYTHON_PN}-subprocess \
"
