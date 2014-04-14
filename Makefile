all : sync
sync: sync.c
	$(CC) $(CCFLAGS) -g -ldl -pthread sync.c -o sync
