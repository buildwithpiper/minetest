set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(project $ENV{PROJECT_ROOT})
set(CMAKE_SYSROOT $ENV{EROOTFS})
set(CMAKE_STAGING_PREFIX $ENV{EROOTFS}/usr/share/minetest)

set(tools /usr/bin)
set(CMAKE_C_COMPILER ${tools}/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER ${tools}/arm-linux-gnueabihf-g++)
set(CMAKE_CXX_FLAGS "-march=armv8-a+crc -mtune=cortex-a53 -mfpu=neon-vfpv4 -mfloat-abi=hard")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

