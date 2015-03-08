#include <ltdl.h>
#include "plugin.h"
#include <string.h>


plugin * get_plugin(char * filename, init_args args){
  lt_dlhandle  ptr = lt_dlopen(filename);

  if (ptr == NULL) return NULL;
  plugin * out = (plugin*)malloc(sizeof(plugin));
  out->dlhandle = ptr;
  out->init 	 =	(S_INIT) 		lt_dlsym(ptr, "init");
  out->listen 	 =	(S_LISTEN) 		lt_dlsym(ptr, "sync_listen");
  out->watch_dir = 	(S_WATCH_DIR) 	lt_dlsym(ptr, "watch_dir");
  out->open		 =	(S_OPEN_FILE) 	lt_dlsym(ptr, "sync_open");
  out->write	 =	(S_WRITE)	lt_dlsym(ptr,"sync_write");
  out->rm		 =	(S_RM)	lt_dlsym(ptr,"sync_rm");
  out->mv		 =	(S_MV)	lt_dlsym(ptr,"sync_mv");
  out->mkdir	 =	(S_MKDIR)	lt_dlsym(ptr,"sync_mkdir");
  out->unload	 =	(S_UNLOAD)	lt_dlsym(ptr,"sync_unload");
  out->prefix	 =  out->init(args);
  out->prefix_len=  strlen(out->prefix);

  return out;
}

plugin ** get_plugins(char * plugin_dir){

}



