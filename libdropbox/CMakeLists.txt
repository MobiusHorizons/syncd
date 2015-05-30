include_directories("${CMAKE_ROOT}")

add_library(dropbox dropbox_api.c)
set_property(TARGET dropbox PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(dropbox LINK_PRIVATE rest json-c)

