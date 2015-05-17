add_library(syncFile MODULE syncfs.c)

add_library(syncDrobBox MODULE dropbox.c)
target_link_libraries(syncDrobBox LINK_PRIVATE dropbox)

add_library(syncGoogleDrive MODULE gdrive.c gdrive_cache.c)
target_link_libraries(syncGoogleDrive LINK_PRIVATE gdrive)

