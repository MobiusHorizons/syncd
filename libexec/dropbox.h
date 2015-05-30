#define _XOPEN_SOURCE 500
#define XOPEN_SOURCE
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../libdropbox/dropbox_api.h"

#define PLUGIN_PREFIX "dropbox://"
#define PLUGIN_PREFIX_LEN 10 // i counted
#include <src/plugin.h>

#ifdef WIN32
	#include "strptime.h"
	#define EXPORT_SHARED __declspec(dllexport)
#else
	#include <sys/wait.h>
	#define EXPORT_SHARED 
#endif

#if defined(_WIN32)
#define URL_OPEN_CMD "start \"\""
#define SILENT_CMD ""
#elif defined(__APPLE__) && defined(__MACH__)
#define URL_OPEN_CMD "open"
#define SILENT_CMD ""
#else
#define URL_OPEN_CMD "sh -c 'xdg-open"
#define SILENT_CMD "' >/dev/null 2&>/dev/null"
#endif

const char * EXPORT_SHARED init            (init_args               );
void         EXPORT_SHARED sync_listen     (int(*)(const char*,int) );
FILE *       EXPORT_SHARED sync_open       (const char*             );
int          EXPORT_SHARED sync_write      (const char*,FILE *      );
int          EXPORT_SHARED sync_rm         (const char*             );
int          EXPORT_SHARED sync_mv         (const char*,const char* );
void         EXPORT_SHARED watch_dir       (const char*             );
int          EXPORT_SHARED sync_mkdir      (const char*             );
void         EXPORT_SHARED sync_unload     (                        );
const char * EXPORT_SHARED get_prefix      (                        );