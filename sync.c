#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <alloca.h>


/*typedef struct {
	
} listenerQueue;
int addListener(listenerQueue lq,functionPointer);*/

int cb(char * path, int mask){
	printf("file %s changed, mask was %.4x\n",path,mask);
}

typedef struct{
	void * fun;
	void * args;
} args_t;

void runner(void* ptr){
	args_t* args = (args_t * ) ptr;
	if (args->fun){
		void (*loader)( int(*)(char*,int)) = args->fun;
		int (*arg)(char*,int) = args->args;
		loader(arg);
	}
		
}

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
	char *testDir1 = alloca(256); //allocate 256 bytes for filename
	strcat(testDir1, getenv("HOME"));
	strcat(testDir1, "/Documents");

	char *testDir2 = alloca(256); //allocate 256 bytes for filename
	strcat(testDir2, getenv("HOME"));
	strcat(testDir2, "/Pictures");

	void ** plugins;
	pthread_t t;
	args_t args;
	int num_plugins = loadPlugins(&plugins);
	int i;
	for ( i = 0; i < num_plugins; i++){
		void (*init)() = dlsym(plugins[i],"init");
		init();
		void (*watch_dir)(const char * ) = dlsym(plugins[i],"watch_dir");
		args.fun = dlsym(plugins[i],"listen");
		watch_dir(testDir1);
		args.args = cb;
		pthread_create(&t,NULL,(void*) &runner,(void*)&args);
		watch_dir(testDir2);
	}
	pthread_join(t,NULL);
	unloadPlugins(plugins,num_plugins);
}
