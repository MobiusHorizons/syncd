#ifndef _SYNCFS_H_
#define _SYNCFS_H_

#include <string.h>
#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <ftw.h>
#undef PLUGIN_PREFIX
#include <src/plugin.h>
/* "readdir" etc. are defined here. */
#include <dirent.h>
/* limits.h defines "PATH_MAX". */
#include <limits.h>
#ifndef PATH_MAX
#include <sys/syslimits.h>
#endif
#define PLUGIN_PREFIX "file://"
#define PLUGIN_PREFIX_LEN 7

#if defined _MSC_VER
#include <direct.h>
#elif defined __GNUC__
#include <sys/types.h>
#include <sys/stat.h>
#endif

// Variables
int(*update_event)(const char*,int);
init_args args;


//Prototypes
void 	add_watch(char *);
void 	watch_dir(char *);
void 	sync_listen(int(*)(const char*,int));
int 	sync_mkdir(char*);
int 	update_file_cache(char*,int);
void 	local_init();
void 	local_unload();

#endif
