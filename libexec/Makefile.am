pkglib_LTLIBRARIES = libsyncfs.la libsyncdb.la libsyncgdrive.la

if USE_INOTIFY
libsyncfs_la_SOURCES = syncfs.c syncfs.h linuxfs.c ../src/json_helper.c ../src/plugin.c
endif

if USE_LIBUV
libsyncfs_la_SOURCES = syncfs.c syncfs.h uvfs.c ../src/json_helper.c ../src/plugin.c
endif

EXTRA_libsyncfs_la_SOURCES = linuxfs.c uvfs.c ../src/plugin.h
libsyncfs_la_LDFLAGS = -avoid-version -module -shared -export-dynamic

libsyncdb_la_SOURCES = dropbox.c dropbox.h ../src/json_helper.c ../src/plugin.c
libsyncdb_la_LIBADD = ../libdropbox/libdropbox.la
libsyncdb_la_LDFLAGS = -avoid-version -module -shared -export-dynamic

libsyncgdrive_la_SOURCES = gdrive.c gdrive.h gdrive_cache.h gdrive_cache.c ../src/json_helper.c ../src/plugin.c
libsyncgdrive_la_LIBADD = ../libgdrive/libgdrive.la
libsyncgdrive_la_LDFLAGS = -avoid-version -module -shared -export-dynamic
