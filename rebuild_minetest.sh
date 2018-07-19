#!/bin/bash
set -o nounset
set -o errtrace
trap 'echo "Aborting due to errexit on line $LINENO. Exit code: $?" >&2' ERR
set -o pipefail

cmake . -DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
	-DLUA_INCLUDE_DIR=${EROOTFS}/usr/include/luajit-2.0 \
        -DENABLE_LUAJIT=YES \
	-DRUN_IN_PLACE=TRUE \
	-DENABLE_GLES=0 \
	-DBUILD_SERVER=NO \
	-DIRRLICHT_SOURCE_DIR=irrlicht/source/Irrlicht/ \
	-DIRRLICHT_LIBRARY=irrlicht/lib/Linux/libIrrlicht.a \
	-DIRRLICHT_INCLUDE_DIR=irrlicht/include/ \
	-DCMAKE_EXE_LINKER_FLAGS=''

make -j $(nproc || sysctl -n hw.ncpu || echo 2)
