#define  _XOPEN_SOURCE 500
#include <libgen.h>
#include "../libgdrive/gdrive_api.h"
#include "../src/os.h"
#include "../src/json_helper.h"

#ifdef WIN32
	#include "strptime.h"
	#define SHARED_EXPORT  __declspec(dllexport)
#else
	#include <sys/wait.h>
	#define SHARED_EXPORT
#endif

#define PLUGIN_PREFIX "gdrive://"
#define PLUGIN_PREFIX_LEN 9
#include "../src/plugin.h"
#include "gdrive_cache.h"


#define __GLOBAL_CLIENT_SECRET  "gcyc89d--P9nUb1KagVeV496"
#define __GLOBAL_CLIENT_ID      "969830472849-93kt0dqjevn8jgr3g6erissiocdhk2fo.apps.googleusercontent.com"
#define REFRESH_TOKEN "1/8obRmFxvhhebWSCYckmw_AfUlfTD-ERnwvoro8tMAKI"

#if defined (WIN32)
#define URL_OPEN_CMD "start \"\""
#define SILENT_CMD ""
#elif defined(__APPLE__) && defined(__MACH__)
#define URL_OPEN_CMD "open"
#define SILENT_CMD "2&>/dev/null"
#else
#define URL_OPEN_CMD "sh -c 'xdg-open"
#define SILENT_CMD "' 2&>/dev/null"
#endif

const char * EXPORT_SHARED init            (init_args               );
void         EXPORT_SHARED sync_listen     (int(*)(const char*,int) );
FILE *       EXPORT_SHARED sync_open_file  (const char*             );
int          EXPORT_SHARED sync_write      (const char*,FILE *      );
int          EXPORT_SHARED sync_rm         (const char*             );
int          EXPORT_SHARED sync_mv         (const char*,const char* );
void         EXPORT_SHARED watch_dir       (const char*             );
int          EXPORT_SHARED sync_mkdir      (const char*             );
void         EXPORT_SHARED sync_unload     (                        );
const char * EXPORT_SHARED get_prefix      (                        );