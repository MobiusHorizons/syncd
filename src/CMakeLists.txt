find_package(LTDL)
include_directories("${CMAKE_CURRENT_LIST_DIR}")
include_directories("${LTDL_INCLUDE_DIRS}")

IF(WIN32)
	find_package(mman)
ENDIF()

add_executable(syncd syncd.c cache.c ipc_semaphore.c shared_memory.c json_helper.c log.c)
target_link_libraries(syncd LINK_PRIVATE ${CMAKE_THREAD_LIBS_INIT} ${LTDL_LIBRARIES} ${JSONC_LIBRARIES} ${MMAN_LIBRARIES})

add_executable(testPlugin testPlugin.c cache.c ipc_semaphore.c shared_memory.c json_helper.c log.c)
target_link_libraries(testPlugin LINK_PRIVATE ${CMAKE_THREAD_LIBS_INIT} ${LTDL_LIBRARIES} ${JSONC_LIBRARIES} ${MMAN_LIBRARIES})
