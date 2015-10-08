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

#include "syncfs.h"
#include <deps/strdup/strdup.h>
#include <uv-version.h>
#include <uv.h>

uv_loop_t *loop;

void local_init(){
  loop = uv_default_loop();
}

void uvfs_updates(uv_fs_event_t *handle, const char *filename, int events, int status) {
  char * fullname = malloc(PLUGIN_PREFIX_LEN + strlen((char*)handle->data) + strlen(filename) + 2);
  sprintf(fullname, "%s%s/%s",PLUGIN_PREFIX, (char * )handle->data, filename);
  printf("caught %s event for file %s\n", events == 1 ? "change":"rename", fullname);
  int mask;
  if ((mask = update_file_cache(fullname + PLUGIN_PREFIX_LEN, 1)) != 0){
    printf("mask = %X\n",mask);
    if (mask & (S_DIR|S_CREATE|S_CLOSE_WRITE) ){
      printf("adding new directory '%s'",fullname+PLUGIN_PREFIX_LEN);
      sync_watch_dir(fullname + PLUGIN_PREFIX_LEN);
    }
    update_event(fullname, mask);
  }
  free(fullname);
}

// using ABI for libuv 1.x
#if UV_VERSION_MAJOR == 1
void fs_add_watch(char* dir_name){
  uv_fs_event_t *fs_event_req = malloc(sizeof(uv_fs_event_t));
  fs_event_req->data = strdup(dir_name);
  uv_fs_event_init(loop, fs_event_req);
  uv_fs_event_start(fs_event_req, uvfs_updates, dir_name, UV_FS_EVENT_RECURSIVE);
}

#elif UV_VERSION_MAJOR == 0

// using ABI for libuv 0.x
void fs_add_watch(char* dir_name){
  // The recursive flag watches subdirectories too.
  uv_fs_event_t *fs_event_req = malloc(sizeof(uv_fs_event_t));
  fs_event_req->data = strdup(dir_name); 
  uv_fs_event_init(loop,fs_event_req, dir_name,uvfs_updates, UV_FS_EVENT_RECURSIVE);
}

#else
#error "Unsupported version for libuv: v" #UV_VERSION_MAJOR ".x"
#endif

void sync_listen(int (*cb)( const char*,int)){
  uv_run(loop, UV_RUN_DEFAULT);
}

void local_unload(){

}
