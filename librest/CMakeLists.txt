find_package(CURL)

include_directories(${CURL_INCLUDE_DIRS})
add_library(rest rest.c buffer.c)
target_link_libraries(rest LINK_PUBLIC ${CURL_LIBRARIES})
