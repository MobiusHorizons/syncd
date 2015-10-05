include_directories("${CMAKE_ROOT}")

#add_executable(dropbox_main dropbox_api.c dropbox_main.c)
#target_link_libraries(dropbox_main LINK_PRIVATE rest ${JSONC_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

add_library(dropbox dropbox_api.c)
set_property(TARGET dropbox PROPERTY POSITION_INDEPENDENT_CODE True)
target_link_libraries(dropbox LINK_PRIVATE rest ${JSONC_LIBRARIES})
