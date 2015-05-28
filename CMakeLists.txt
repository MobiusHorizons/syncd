cmake_minimum_required (VERSION 2.6)
project (Syncd)

# Include findPackage and findHeader
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/.cmake-modules")
include(CheckIncludeFiles)
include(CheckFunctionExists)

find_package(JSONC)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)

include_directories(${JSONC_INCLUDE_DIRS})

check_include_files("inotify.h" HAVE_INOTIFY_H)
check_include_files("err.h" HAVE_ERR_H)
check_include_files("sys/wait.h" HAVE_WAIT_H)
check_function_exists(fork HAVE_FORK)

include_directories("${CMAKE_BINARY_DIR}")
include_directories("${CMAKE_CURRENT_LIST_DIR}")

set(PLUGIN_DIR "${CMAKE_INSTALL_PREFIX}/lib/syncd/plugins")
add_subdirectory(librest)
add_subdirectory(libdropbox)
add_subdirectory(libgdrive)
add_subdirectory(libexec)
add_subdirectory(src)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake-config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
