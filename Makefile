CC:= clang
CFLAGS := -g

all : sync plugins
sync: sync.c
	$(CC) $(CFLAGS) -fPIC -s -ldl -pthread -lpython2.7 sync.c -o sync

plugins :
	$(MAKE) -C plugins
