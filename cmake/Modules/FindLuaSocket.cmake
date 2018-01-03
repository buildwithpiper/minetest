mark_as_advanced(LUASOCKET_LIBRARY LUASOCKET_INCLUDE_DIR)

message(STATUS "Using bundled luasocket library.")
set(LUASOCKET_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/luasocket)
set(LUASOCKET_LIBRARY luasocket)
add_subdirectory(lib/luasocket)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LUASOCKET DEFAULT_MSG LUASOCKET_LIBRARY LUASOCKET_INCLUDE_DIR)
