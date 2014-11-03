CCFLAGS := -ggdb
ifeq ($(CC),cc)
SL :=.so
OBJ :=.o
CCFLAGS := $(CCFLAGS) -fPIC
PKG_CONFIG :=pkg-config
else
CCFLAGS := $(CCFLAGS) -Icurl/include -DCURL_STATICLIB
OBJ:=.wo
SL:=.dll
EXE:=.exe
LIBS:= $(shell $(PKG_CONFIG) --static --libs libcurl )
endif
all: dropbox$(EXE) 
#libdropbox$(SL)

dropbox: dropbox_api.o dropbox_main.o librest.so
	$(CC) $(CCFLAGS) -o dropbox dropbox_api.o dropbox_main.o -Llibrest/ -pthread -lrest -ljson-c -lcurl 

dropbox.exe: dropbox_api.wo dropbox_main.wo
	$(CC) -o dropbox.exe dropbox_api.wo dropbox_main.wo -static -lpthread -ljson-c -liconv -Llibrest -Wl,-Bdynamic,-lrest

#i686-w64-mingw32-gcc -o dropbox.exe dropbox_api.o dropbox_main.o -pthread -static -ljson-c -Lcurl/lib -lrtmp $(mingw32-pkg-config --libs --static libcurl) -lrtmp -lssl -lwinmm -Wl,-Bdynamic -lws2_32

#dropbox.exe: dapi.o dropbox_main.o
#	$(CC) $(CCFLAGS) -o dropbox.exe dropbox_api.o dropbox_main.o -ljson-c -pthread $(LIBS)

libdropbox.so: dropbox_api.o
	$(LD) -shared -o libdropbox.so dropbox_api.o  

libdropbox.dll: dropbox_api.o
	$(CC) -shared -o $@ $< -static -ljson-c -lpthread -Lcurl/lib -lrtmp $(LIBS) -lrtmp -lssl -lwinmm -Wl,--out-implib,$@.a,-Bdynamic -lws2_32

dropbox_api$(OBJ): dropbox_api.c dropbox_api.h dropbox_urls.h buffer.h
	$(CC) $(CCFLAGS) -o $@ -c dropbox_api.c

dropbox_main$(OBJ): dropbox_main.c dropbox_api.h
	$(CC) $(CCFLAGS) -o $@ -c dropbox_main.c

librest.so: 
	make install -C librest

clean: 
	rm -f *.o *.dll dropbox.exe dropbox *.so *.a *~ *.swp
