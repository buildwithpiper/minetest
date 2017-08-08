#!/bin/bash
set -o nounset
set -o errtrace
trap 'echo "Aborting due to errexit on line $LINENO. Exit code: $?" >&2' ERR
set -o pipefail

cmake . -DCMAKE_BUILD_TYPE=Release \
	-DLUA_INCLUDE_DIR=/usr/local/include/luajit-2.1 \
        -DENABLE_LUAJIT=YES \
	-DRUN_IN_PLACE=TRUE \
	-DENABLE_GLES=1 \
    	-DEGL_INCLUDE_DIR=/usr/include/EGL \
	-DEGL_LIBRARY=/usr/lib/arm-linux-gnueabihf/libEGL.so.1 \
	-DOPENGLES2_INCLUDE_DIR=/usr/include \
	-DBUILD_SERVER=NO \
	-DIRRLICHT_SOURCE_DIR=irrlicht/source/Irrlicht/ \
	-DIRRLICHT_LIBRARY=irrlicht/lib/Linux/libIrrlicht.a \
	-DIRRLICHT_INCLUDE_DIR=irrlicht/include/ \
	-DCMAKE_EXE_LINKER_FLAGS=-lEGL

make -j $(nproc || sysctl -n hw.ncpu || echo 2)
