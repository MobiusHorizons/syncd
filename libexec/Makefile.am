pkglib_LTLIBRARIES = libsyncfs.la libsyncdb.la libsyncgdrive.la

libsyncfs_la_SOURCES = syncfs.c ../src/json_helper.c
libsyncfs_la_LDFLAGS = -avoid-version -module -shared -export-dynamic

libsyncdb_la_SOURCES = dropbox.c ../src/json_helper.c
libsyncdb_la_LIBADD = ../libdropbox/libdropbox.la
libsyncdb_la_LDFLAGS = -avoid-version -module -shared -export-dynamic

libsyncgdrive_la_SOURCES = gdrive.c gdrive_cache.h gdrive_cache.c ../src/json_helper.c
libsyncgdrive_la_LIBADD = ../libgdrive/libgdrive.la
libsyncgdrive_la_LDFLAGS = -avoid-version -module -shared -export-dynamic
