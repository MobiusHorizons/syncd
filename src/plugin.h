#ifndef __PLUGIN_H_
#define __PLUGIN_H_

#include "json_helper.h"
#include "cache.h"
#include <stdio.h>
#include <ltdl.h>
#define S_CLOSE_WRITE   0x00000008
#define S_CLOSE_NOWRITE 0x00000010
#define S_CLOSE         (S_CLOSE_WRITE | S_CLOSE_NOWRITE)
#define S_OPEN          0x00000020
#define S_MOVED_FROM    0x00000040
#define S_MOVED_TO	    0x00000080
#define S_MOVE		      (S_MOVED_FROM|S_MOVED_TO)
#define S_CREATE		    0x00000100
#define S_DELETE		    0x00000200
#define S_DIR		        0x40000000

typedef struct {
  utilities utils;
  int(*event_callback)(const char*,int);
  //JSON json;
} init_args;

typedef void 	(*S_LISTEN    )	(int(*)(const char*,int) );
typedef FILE* (*S_OPEN_FILE )	(const char*             );
typedef void 	(*S_CLOSE_FILE)	(FILE*                   );
typedef int 	(*S_WRITE     )	(const char*,FILE *      );
typedef int 	(*S_RM        )	(const char*             );
typedef int 	(*S_MV        )	(const char*,const char* );
typedef char* (*S_INIT      )	(init_args               );
typedef void 	(*S_WATCH_DIR )	(const char*             );
typedef int   (*S_MKDIR     )	(const char*             );
typedef void	(*S_UNLOAD    ) (                        );


typedef struct{
  /* functions */
  S_INIT      init;
  S_LISTEN    listen;
  S_WATCH_DIR watch_dir;
  S_OPEN_FILE open;
  S_WRITE     write;
  S_RM        rm;
  S_MV        mv;
  S_MKDIR     mkdir;
  S_UNLOAD    unload;
  /* properties */
  char *      prefix;
  int         prefix_len;
  lt_dlhandle dlhandle;
} plugin;


//int loadPlugins(plugin ***plugins,
//                char     *dir);
#endif
