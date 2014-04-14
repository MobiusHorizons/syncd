CFLAGS := -g 
all: libsyncfs.so
libsyncfs.so : syncfs.c

%.o: %.c
	$(CC) -fPIC -c $(CFLAGS) $(CPPFLAGS) $< -o $@

lib%.so: %.o
	$(CC) -shared -Wl,-soname,$@.1 -o $@.1.0.1  $<
