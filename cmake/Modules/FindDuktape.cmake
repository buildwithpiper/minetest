mark_as_advanced(DUKTAPE_LIBRARY DUKTAPE_INCLUDE_DIR)


message(STATUS "Using duktape library.")
set(DUKTAPE_LIBRARY duktape)
set(DUKTAPE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/duktape)
add_subdirectory(lib/duktape)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(duktape DEFAULT_MSG DUKTAPE_LIBRARY DUKTAPE_INCLUDE_DIR)
