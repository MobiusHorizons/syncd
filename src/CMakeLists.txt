include_directories("${CMAKE_ROOT}")
include_directories("${CMAKE_CURRENT_LIST_DIR}")
add_executable(syncd syncd.c cache.c ipc_semaphore.c shared_memory.c json_helper.c log.c os.h plugin.h)

