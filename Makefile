PLUGINS := fs db 
CFLAGS := -ggdb $(shell pkg-config --cflags json-c) 
EXEEXT:=
ifeq ($(CC),cc)
PLUGIN_EXT :=so
CLIBS:=  -pthread -ldl $(shell pkg-config --libs json-c)
CFLAGS := $(CFLAGS) -fPIC
OBJ:=.lo
else
PLUGIN_EXT :=dll
CLIBS:= -pthread #$(shell pkg-config --libs python) -pthread
EXEEXT:=.exe
OBJ:=.wo
endif
PLUGIN_LIBS := $(foreach P,$(PLUGINS), plugins/libsync$(P).$(PLUGIN_EXT))

all : sync$(EXEEXT) plugins

sync$(EXEEXT): sync$(OBJ) cache$(OBJ)
	$(CC) $(CLIBS) sync$(OBJ) cache$(OBJ) -o sync$(EXEEXT)

sync$(OBJ): sync.c cache.h
	$(CC) -c $(CFLAGS) sync.c -o sync$(OBJ)

cache$(OBJ): cache.c cache.h
	$(CC) -c $(CFLAGS) cache.c -o cache$(OBJ)

plugins : $(PLUGIN_LIBS)

plugins/%.$(PLUGIN_EXT) :
	$(MAKE) $*.$(PLUGIN_EXT) -C plugins 

