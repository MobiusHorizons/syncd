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
      watch_dir(fullname + PLUGIN_PREFIX_LEN);
    }
    update_event(fullname, mask);
  }
  free(fullname);
}

void add_watch(char * dir_name){
  uv_fs_event_t *fs_event_req = malloc(sizeof(uv_fs_event_t));
  fs_event_req->data = strdup(dir_name);
//  uv_fs_event_init(loop, fs_event_req);
  // The recursive flag watches subdirectories too.
//  uv_fs_event_start(fs_event_req, uvfs_updates, dir_name, UV_FS_EVENT_RECURSIVE);
  uv_fs_event_init(loop,fs_event_req,dir_name,uvfs_updates,UV_FS_EVENT_RECURSIVE);

}

void sync_listen(int (*cb)( const char*,int)){
  uv_run(loop, UV_RUN_DEFAULT);
}


void local_unload(){

}
