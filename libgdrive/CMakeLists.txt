add_library(gdrive gdrive_api.c)
set_property(TARGET gdrive PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(gdrive LINK_PRIVATE rest json-c)
