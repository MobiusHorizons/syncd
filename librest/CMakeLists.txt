find_package(CURL)
set(POSITION_INDEPENDENT_CODE TRUE)

include_directories(${CURL_INCLUDE_DIRS})
add_library(rest rest.c buffer.c)
set_property(TARGET rest PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(rest LINK_PUBLIC ${CURL_LIBRARIES})

if(WIN32)
install()
endif()
