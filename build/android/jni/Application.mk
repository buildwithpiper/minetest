# NDK_TOOLCHAIN_VERSION := clang3.8

APP_PLATFORM := android-14
APP_ABI := armeabi-v7a
APP_STL := c++_shared
APP_DEPRECATED_HEADERS := true
APP_MODULES := minetest

APP_CLAFGS += -mfloat-abi=softfp -mfpu=vfpv3 -O3
APP_CPPFLAGS += -fexceptions -std=c++11 -frtti
APP_LDFLAGS += -lstdc++
LOCAL_C_INCLUDES += ${ANDROID_NDK}/sources/cxx-stl/llvm-libc++/include
