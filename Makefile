CFLAGS := -ggdb 
EXEEXT:=
ifeq ($(CC),cc)
CLIBS:=  -pthread -ldl 
CFLAGS := $(CFLAGS) -fPIC

else
CLIBS:= -pthread #$(shell pkg-config --libs python) -pthread
EXEEXT:=.exe
endif

all : sync$(EXEEXT) plugins/
sync$(EXEEXT): sync.c
	$(CC) $(CFLAGS) sync.c $(CLIBS) -o sync$(EXEEXT)

plugins/ : plugins/*.so
	$(MAKE) -C plugins
