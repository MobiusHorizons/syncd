CC:= gcc
CFLAGS := 

all : sync plugins/
sync: sync.c
	$(CC) $(CFLAGS) -fPIC -ggdb -ldl -pthread -lpython2.7 sync.c -o sync

plugins/ : plugins/*.so
	$(MAKE) -C plugins
