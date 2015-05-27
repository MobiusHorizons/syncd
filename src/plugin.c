//#include <ltdl.h>
#include "plugin.h"
#include <stdio.h>
#include <string.h>
#include "os.h"

bool __sigterm__ = false;

void set_sigterm(int signum){
    __sigterm__ = true;
}

bool plugin_get_sigterm(){
	return __sigterm__;
}

void plugin_catch_events(){
#ifndef WIN32
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = set_sigterm;
    sigaction(SIGTERM, &action, NULL);
#endif
}
  
 /* 
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

int loadPlugins(plugin *** return_plugins, char * dir_name){
	plugin ** plugins = NULL;
	plugin * p;
	init_args args;
	args.utils = get_utility_functions();
	int num_plugins = 0, i=0;
	DIR * dp;
	struct dirent *ep;
    printf("looking for plugins in %s\n", dir_name);
	dp = opendir(dir_name); //TODO this should pull from config.h
	if (dp != NULL)
	{
		while ((ep = readdir (dp))){
#if defined(UNIX)
			if(ep->d_type == DT_REG){
#endif
				char * filename = malloc(strlen(ep->d_name) + strlen(dir_name)+2);
				sprintf(filename,"%s/%s",dir_name,ep->d_name);
				p = get_plugin(filename, args);
				if (p != NULL){
					plugins = realloc(plugins,(num_plugins+1) * sizeof(plugin*) );
					printf ("prefix for plugin %d is '%s'\n",num_plugins,p->prefix);
					plugins[num_plugins] = p;
					num_plugins ++ ;
				} else {
					printf ("%s is not a valid plugin\n",filename);
				}
				free(filename);
#if defined(UNIX)
			}
#endif
		}
		closedir (dp);
	}
	*return_plugins = plugins;
	return num_plugins;
}


*/