APP_PLATFORM := android-14
APP_ABI := armeabi-v7a
APP_STL := c++_shared
NDK_TOOLCHAIN_VERSION := 4.9
APP_DEPRECATED_HEADERS := true

APP_CLAFGS += -mfloat-abi=softfp -mfpu=vfpv3 -O3
APP_CPPFLAGS += -fexceptions -std=c++11
APP_LDFLAGS += -lstdc++
