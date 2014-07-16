PLUGINS := fs db gdrive
CFLAGS := -ggdb 
EXEEXT:=
ifeq ($(CC),cc)
PLUGIN_EXT :=so
CLIBS:=  -pthread -ldl 
CFLAGS := $(CFLAGS) -fPIC

else
PLUGIN_EXT :=dll
CLIBS:= -pthread #$(shell pkg-config --libs python) -pthread
EXEEXT:=.exe
endif
PLUGIN_LIBS := $(foreach P,$(PLUGINS), plugins/libsync$(P).$(PLUGIN_EXT))

all : sync$(EXEEXT) plugins
sync$(EXEEXT): sync.c
	$(CC) $(CFLAGS) sync.c $(CLIBS) -o sync$(EXEEXT)

plugins : $(PLUGIN_LIBS)

plugins/%.$(PLUGIN_EXT) :
	$(MAKE) $*.$(PLUGIN_EXT) -C plugins 

