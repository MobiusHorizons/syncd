include_directories("${CMAKE_ROOT}")

#add_executable(gdrive_main gdrive_main.c gdrive_api.c)
#target_link_libraries(gdrive_main LINK_PRIVATE rest ${JSONC_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_library(gdrive gdrive_api.c)
set_property(TARGET gdrive PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(gdrive LINK_PRIVATE rest ${JSONC_LIBRARIES})
