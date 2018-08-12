#!/bin/bash
set -o nounset
set -o errtrace
trap 'echo "Aborting due to errexit on line $LINENO. Exit code: $?" >&2' ERR
set -o pipefail

EGL_MESA=0
EGL_BROADCOM=0

if [ $EGL_BROADCOM -eq 1 ]; then
	EGL_INC=${EROOTFS}/opt/vc/include
	EGL_LIB=${EROOTFS}/opt/vc/lib
	EGL_LINK="-L${EGL_LIB} -lbrcmEGL -lbrcmGLESv2 -lbcm_host -lvcos -lvchiq_arm"
elif [ $EGL_MESA -eq 1 ]; then
	EGL_INC=${EROOTFS}/usr/include
	EGL_LIB=${EROOTFS}/usr/lib
	EGL_LINK="-L${EGL_LIB} -lEGL -lGLESv2"
else
	EGL_INC=${EROOTFS}/usr/include
	EGL_LIB=${EROOTFS}/usr/lib
	EGL_LINK=""
fi

cmake . -DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
	-DLUA_INCLUDE_DIR=${EROOTFS}/usr/include/luajit-2.0 \
        -DENABLE_LUAJIT=YES \
	-DRUN_IN_PLACE=TRUE \
	-DENABLE_GLES=0 \
	-DEGL_INCLUDE_DIR=$EGL_INC \
	-DOPENGLES2_INCLUDE_DIR=$EGL_INC \
	-DBUILD_SERVER=NO \
	-DIRRLICHT_SOURCE_DIR=irrlicht/source/Irrlicht/ \
	-DIRRLICHT_LIBRARY=irrlicht/lib/Linux/libIrrlicht.a \
	-DIRRLICHT_INCLUDE_DIR=irrlicht/include/ \
	-DCMAKE_EXE_LINKER_FLAGS="${EGL_LINK}"

make -j $(nproc || sysctl -n hw.ncpu || echo 2)
