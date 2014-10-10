CFLAGS := -Ilibdropbox/ -ggdb -fno-stack-protector
all: libsyncfs.so libsyncgdrive.so
libsyncfs.so : syncfs.c ../json_helper.lo


#libsyncdb.so: dropbox_c.o dropbox_py.o
#	ld  -shared -o libsyncdb.so dropbox_c.o dropbox_py.o -lpython2.7
#

libsyncdb.so: dropbox.o libdropbox/dropbox_api.o
	ld -shared -o libsyncdb.so dropbox.o libdropbox/dropbox_api.o -lcurl -ljson-c -lpthread -lrest ../json_helper.lo

libsyncgdrive.so: gdrive.o libgdrive/gdrive_api.lo
	ld -shared -o libsyncgdrive.so gdrive.o libgdrive/gdrive_api.lo -lcurl -ljson-c -lpthread -lrest

%.o: %.c
	$(CC) -O2 -fPIC -c $(CFLAGS) $(CPPFLAGS) $< -o $@

lib%.so: %.o
	ld -shared   -lc -o $@  $< ../json_helper.lo
lib%.dll:%.o
	$(CC) -shared -o $@ $< -Wl,--out-implib,$@.a
clean:
	rm -f dropbox_py.c dropbox_py.h  *.o *.so
