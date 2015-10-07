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

#include <sys/inotify.h>
#include <deps/strdup/strdup.h>
#include "syncfs.h"

static int inotify_fd;
static char** watchpoints;
static int num_watchpoints;
static int watchpoints_size;

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN	(1024 * (EVENT_SIZE + 16))

void fs_add_watch(char * dir){
  int wp  = inotify_add_watch( inotify_fd, dir, IN_CREATE | IN_DELETE | IN_CLOSE_WRITE | IN_MOVE);
  args.log(LOGARGS,"adding directory: %s, wp = %d\n",dir,wp);
  char * dir_local = strdup(dir);
    while (wp >= watchpoints_size) {
        watchpoints_size *= 2;
        watchpoints = realloc(watchpoints, watchpoints_size * sizeof(char*));
    }
  watchpoints[wp] = dir_local;
  num_watchpoints = wp;
}

void local_init(){
  num_watchpoints = 0;
  watchpoints_size = 1;
  watchpoints = (char **) malloc(sizeof(char*));
  //num_open_files = 0;
  inotify_fd = inotify_init();
  if (inotify_fd < 0){
    perror("inotify_init");
  }
}

void local_unload(){
    free(watchpoints);
}

void sync_listen(int (*cb)( const char*,int)){
  CATCH_EVENTS
  update_event = cb;
  int length;
  char buffer[EVENT_BUF_LEN];
  int i;
  while(1){
    i = 0;
    length = read(inotify_fd,buffer,EVENT_BUF_LEN);
    if (length < 0)
      perror("read");
      while( i < length){
        struct inotify_event * event = (struct inotify_event * ) &buffer[i];
        if (event->len){
          char * fp  = (char * ) malloc(PLUGIN_PREFIX_LEN + event->len + strlen(watchpoints[event->wd])+1);
          sprintf(fp,"%s%s/%s",PLUGIN_PREFIX,watchpoints[event->wd],event->name);
          char * filename = fp + PLUGIN_PREFIX_LEN;
          update_file_cache(filename,1);
          if ((event->mask & IN_CREATE) && (event->mask & IN_ISDIR)){ // new directory created
            args.log(LOGARGS,"%.4x = %.4x\n",event->mask, (IN_CREATE|IN_ISDIR));
            fs_add_watch(fp + PLUGIN_PREFIX_LEN);
          }
          if (cb != NULL) cb(fp,event->mask);
          free(fp);
        }
        i += EVENT_SIZE + event->len;
      }
      CLEAN_BREAK
    }
  }
