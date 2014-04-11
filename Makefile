all : sync
sync: sync.c
	$(CC) $(CCFLAGS) -ldl -pthread sync.c -o sync
