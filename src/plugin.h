/* Copyright (c) 2014 Paul Martin & Brian Cole
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __PLUGIN_H_
#define __PLUGIN_H_

#include "json_helper.h"
#include "cache.h"
#include "log.h"
#include <stdio.h>
#include <ltdl.h>
#include <signal.h>

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
  LOGGING_LOG log;
  LOGGING_STDOUT stdout;
  LOGGING_STDERR stderr;
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
#ifdef PLUGIN_PREFIX
  bool __sigterm__ = false;
  #define CATCH_EVENTS __catch_events();
  void __set_sigterm(int signum){
    __sigterm__ = true;
  }
  void __catch_events(){
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = __set_sigterm;
    sigaction(SIGTERM, &action, NULL);
  }
  #define CLEAN_BREAK  if (__sigterm__) return;
#endif

#endif
