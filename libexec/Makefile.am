pkglib_LTLIBRARIES = libsyncdb.la

#libsyncfs_la_SOURCES = syncfs.c ../src/json_helper.c
#libsyncfs_la_LDFLAGS = -avoid-version -module -shared -export-dynamic

libsyncdb_la_SOURCES = dropbox.c ../src/json_helper.c
libsyncdb_la_LIBADD = ../libdropbox/libdropbox.la
libsyncdb_la_LDFLAGS = -avoid-version -module -shared -export-dynamic
