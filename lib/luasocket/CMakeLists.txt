find_package(Lua REQUIRED)

set(LOVE_SRC_3P_LUASOCKET_ROOT
	luasocket.cpp
	luasocket.h
)

set(LOVE_SRC_3P_LUASOCKET_LIBLUASOCKET
	libluasocket/auxiliar.c
	libluasocket/auxiliar.h
	libluasocket/buffer.c
	libluasocket/buffer.h
	libluasocket/compat.c
	libluasocket/compat.h
	libluasocket/except.c
	libluasocket/except.h
	libluasocket/ftp.lua.h
	libluasocket/headers.lua.h
	libluasocket/http.lua.h
	libluasocket/inet.c
	libluasocket/inet.h
	libluasocket/io.c
	libluasocket/io.h
	libluasocket/ltn12.lua.h
	libluasocket/luasocket.c
	libluasocket/luasocket.h
	libluasocket/mbox.lua.h
	libluasocket/mime.c
	libluasocket/mime.h
	libluasocket/mime.lua.h
	libluasocket/options.c
	libluasocket/options.h
	libluasocket/pierror.h
	libluasocket/select.c
	libluasocket/select.h
	libluasocket/smtp.lua.h
	libluasocket/socket.h
	libluasocket/socket.lua.h
	libluasocket/tcp.c
	libluasocket/tcp.h
	libluasocket/timeout.c
	libluasocket/timeout.h
	libluasocket/tp.lua.h
	libluasocket/udp.c
	libluasocket/udp.h
	libluasocket/url.lua.h
)

set(LOVE_LINK_L3P_LUASOCKET_LIBLUASOCKET)

if(LUASOCKET_INET_PTON)
	add_definitions(-DLUASOCKET_INET_PTON)
endif()

if(MSVC OR WIN32)
	set(LOVE_SRC_3P_LUASOCKET_LIBLUASOCKET
		${LOVE_SRC_3P_LUASOCKET_LIBLUASOCKET}
		libluasocket/wsocket.c
		libluasocket/wsocket.h
	)

	set(LOVE_LINK_L3P_LUASOCKET_LIBLUASOCKET
		${LOVE_LINK_L3P_LUASOCKET_LIBLUASOCKET}
		ws2_32.lib
	)
else()
	set(LOVE_SRC_3P_LUASOCKET_LIBLUASOCKET
		${LOVE_SRC_3P_LUASOCKET_LIBLUASOCKET}
		libluasocket/serial.c
		libluasocket/unix.c
		libluasocket/unix.h
		libluasocket/unixtcp.c
		libluasocket/unixtcp.h
		libluasocket/unixudp.c
		libluasocket/unixudp.h
		libluasocket/usocket.c
		libluasocket/usocket.h
	)
endif()

set(LOVE_SRC_3P_LUASOCKET
	${LOVE_SRC_3P_LUASOCKET_ROOT}
	${LOVE_SRC_3P_LUASOCKET_LIBLUASOCKET}
)
include_directories(
	${LUA_INCLUDE_DIR}
)
add_library(luasocket ${LOVE_SRC_3P_LUASOCKET})
target_link_libraries(luasocket ${LOVE_LINK_L3P_LUASOCKET_LIBLUASOCKET})
