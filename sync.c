#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>

/*typedef struct {
	
} listenerQueue;
int addListener(listenerQueue lq,functionPointer);*/

int loadPlugins(void ***return_plugins){
	void ** plugins = NULL;
	int num_plugins = 0, i=0;
	DIR * dp;
	struct dirent *ep;
	dp = opendir("./plugins/");
	if (dp != NULL)
  	{
    		while ((ep = readdir (dp))){
      			if(ep->d_type == DT_REG){
				char * filename = malloc(strlen(ep->d_name) + strlen("plugins/")+1);
				sprintf(filename,"plugins/%s",ep->d_name);
				printf("trying to load %s ",filename);
				void * plugin = dlopen(filename, RTLD_LAZY);
				if (plugin != NULL){
					plugins = realloc(plugins,(num_plugins+1) * sizeof(void*) );
					plugins[num_plugins] = plugin;
					num_plugins ++ ;
					printf ("SUCCESS\n");
				} else{
					printf("FAIL\n");
				}
				free(filename);
			}
		}
		closedir (dp);
  	}
	*return_plugins = plugins;
	return num_plugins;
}

void unloadPlugins(void **plugins, int num){
	int i;
	for (i = 0; i < num; i++){
		dlclose(plugins[i]);
	}
	free(plugins);
}

int main(int argc, char** argv){
	void ** plugins;
	int i;
	int num_plugins = loadPlugins(&plugins);
	for ( i = 0; i < num_plugins; i++){
		void (*init)() = dlsym(plugins[i],"init");
		init();
	}
	unloadPlugins(plugins,num_plugins);
}
