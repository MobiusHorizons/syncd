cmake_minimum_required (VERSION 2.6)
project (Syncd)

include_directories("${CMAKE_CURRENT_LIST_DIR}")
add_subdirectory(librest)
add_subdirectory(libdropbox)
add_subdirectory(libgdrive)
add_subdirectory(libexec)
add_subdirectory(src)
