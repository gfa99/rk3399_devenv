#!/bin/bash

#BUILD_TYPE=Release
BUILD_TYPE=Debug

ROOT_PWD=$( cd "$( dirname $0 )" && cd -P "$( dirname "$SOURCE" )" && pwd )

BUILD_DIR=${ROOT_PWD}/build/build_osx_x86

if [[ ! -d "${BUILD_DIR}" ]]; then
  mkdir -p ${BUILD_DIR}
fi

cd ${BUILD_DIR}
cmake ../.. \
    -G "Unix Makefiles" \
    -DCMAKE_TOOLCHAIN_FILE=${ROOT_PWD}/cmake/x86.darwin.cmake \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_INSTALL_PREFIX=${ROOT_PWD}/install/rockx_osx_x86
make -j4
make install
cd -
