add_library(libdgrive gdrive_api.c)
target_link_libraries(libdgrive LINK_PUBLIC librest json-c)
