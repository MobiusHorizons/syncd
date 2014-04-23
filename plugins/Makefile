CFLAGS := -ggdb 
all: libsyncfs.so libsyncdb.so
libsyncfs.so : syncfs.c


dropbox_py.o: dropbox_py.c
	gcc $(CFLAGS) -s -fPIC -c dropbox_py.c -I/usr/include/python2.7

dropbox_c.o: dropbox_c.c dropbox_py.c
	gcc $(CFLAGS) -s -fPIC -c dropbox_c.c -I/usr/include/python2.7

dropbox_py.c: dropbox_py.pyx
	cython dropbox_py.pyx 

libsyncdb.so: dropbox_c.o dropbox_py.o
	ld  -shared -o libsyncdb.so dropbox_c.o dropbox_py.o -lpython2.7

%.o: %.c
	$(CC) -fPIC -c $(CFLAGS) $(CPPFLAGS) $< -o $@

lib%.so: %.o
	ld -shared  -o $@  $<
clean:
	rm -f dropbox_py.c dropbox_py.h  *.o *.so
