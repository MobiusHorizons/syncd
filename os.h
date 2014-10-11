
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "cache.h"

#if defined(WIN32)
	#include<windows.h>
	#include<malloc.h>
	#define RTLD_LAZY 0
	
	typedef HMODULE Library;
	Library dlopen(const char* path,int mask){
		return LoadLibrary(path);
	}
	FARPROC dlsym(Library lib, const char* name){
		return GetProcAddress(lib,name);
	}
	void dlclose(Library lib){
		FreeLibrary(lib);
	}
    int fork(){
		return 1; // cludge it for now
	}
#else
	#include <alloca.h>
	#include <dlfcn.h>
	#include <unistd.h>
	typedef void* Library ;

#endif



#define S_CLOSE_WRITE	0x00000008
#define S_CLOSE_NOWRITE	0x00000010
#define S_CLOSE		(S_CLOSE_WRITE | S_CLOSE_NOWRITE)
#define S_OPEN 		0x00000020
#define S_MOVED_FROM	0x00000040
#define S_MOVED_TO	0x00000080
#define S_MOVE		(S_MOVED_FROM|S_MOVED_TO)
#define S_CREATE		0x00000100
#define S_DELETE		0x00000200
#define S_ISDIR		0x40000000

typedef void 	(*S_LISTEN    )	(int(*)(const char*,int) );
typedef FILE* 	(*S_OPEN_FILE )	(const char*             );
typedef void 	(*S_CLOSE_FILE)	(FILE*                   );
typedef int 	(*S_WRITE     )	(const char*,FILE *      );
typedef int 	(*S_RM        )	(const char*             );		
typedef int 	(*S_MV        )	(const char*,const char* );
typedef char* 	(*S_INIT      )	(utilities               );
typedef void 	(*S_WATCH_DIR )	(const char*             );
typedef int     (*S_MKDIR     )	(const char*             );
typedef void	(*S_UNLOAD    ) (                        );
