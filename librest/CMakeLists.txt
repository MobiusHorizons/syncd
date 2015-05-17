add_library(rest rest.c buffer.c)
set_property(TARGET rest PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(rest LINK_PUBLIC curl)
