add_library(librest rest.c buffer.c)
target_link_libraries(librest LINK_PUBLIC curl)
