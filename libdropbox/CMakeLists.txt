include_directories("${CMAKE_ROOT}")
set(POSITION_INDEPENDENT_CODE TRUE)

#add_executable(dropbox_main dropbox_api.c dropbox_main.c)
#target_link_libraries(dropbox_main LINK_PRIVATE rest ${JSONC_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

add_library(dropbox dropbox_api.c)
target_link_libraries(dropbox LINK_PRIVATE rest ${JSONC_LIBRARIES})
