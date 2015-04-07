CCFLAGS:= -ggdb

ifeq ($(CC),cc)
OBJ:=.lo
EXE:=
SO:=.so
CCFLAGS:=$(CCFLAGS) -fPIC
else
OBJ:=.wo
EXE:=.exe
SO:=.dll
endif

all: gdrive$(EXE)


gdrive: gdrive_api.lo gdrive_main.lo
	$(CC) $(CCFLAGS) -o gdrive gdrive_api.lo gdrive_main.lo -lrest -lcurl -ljson-c

gdrive.exe : gdrive_api.wo gdrive_main.wo
	$(CC) -o gdrive.exe gdrive_api.wo gdrive_main.wo -static -lpthread -ljson-c -liconv -L../librest -Wl,-Bdynamic,-lrest

gdrive_api$(OBJ) : gdrive_api.c gdrive_api.h
	$(CC) $(CCFLAGS) -c -o $@ gdrive_api.c

gdrive_main$(OBJ): gdrive_api.c gdrive_api.h
	$(CC) $(CCFLAGS) -c -o $@ gdrive_main.c
