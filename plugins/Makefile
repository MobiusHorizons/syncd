CFLAGS := -Ilibdropbox/ -ggdb 
all: libsyncfs.so libsyncgdrive.so
libsyncfs.so : syncfs.c


#libsyncdb.so: dropbox_c.o dropbox_py.o
#	ld  -shared -o libsyncdb.so dropbox_c.o dropbox_py.o -lpython2.7
#

libsyncdb.so: dropbox.o libdropbox/dropbox_api.o
	ld -shared -o libsyncdb.so dropbox.o libdropbox/dropbox_api.o -lcurl -ljson-c -lpthread -lrest

libsyncgdrive.so: gdrive.o libgdrive/gdrive_api.lo
	ld -shared -o libsyncgdrive.so gdrive.o libgdrive/gdrive_api.lo -lcurl -ljson-c -lpthread -lrest

%.o: %.c
	$(CC) -fPIC -c $(CFLAGS) $(CPPFLAGS) $< -o $@

lib%.so: %.o
	ld -shared  -o $@  $<
lib%.dll:%.o
	$(CC) -shared -o $@ $< -Wl,--out-implib,$@.a
clean:
	rm -f dropbox_py.c dropbox_py.h  *.o *.so
