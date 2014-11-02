AUTOMAKE_OPTIONS = foreign

CFLAGS=-Wall -I/usr/local/include -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\""
LDFLAGS=-L/usr/local/lib -lcurl -lpthread -ldl

bin_PROGRAMS = sync
sync_SOURCES = sync.c cache.c cache.h json_helper.c json_helper.h
