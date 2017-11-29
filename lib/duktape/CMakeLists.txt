if(MSVC)
	set(CMAKE_C_FLAGS_RELEASE "/MT /O2 /Ob2 /D NDEBUG")
endif()

add_library(duktape duktape.c)
target_link_libraries(duktape)

