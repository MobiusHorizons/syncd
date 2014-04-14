#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>


/*typedef struct {
	
} listenerQueue;
int addListener(listenerQueue lq,functionPointer);*/

#define CLOSE_WRITE	0x00000008
#define CLOSE_NOWRITE	0x00000010
#define CLOSE		(CLOSE_WRITE | CLOSE_NOWRITE)
#define OPEN 		0x00000020
#define MOVED_FROM	0x00000040
#define MOVED_TO	0x00000080
#define MOVE		(MOVED_FROM|MOVED_TO)
#define CREATE		0x00000100
#define DELETE		0x00000200
#define ISDIR		0x40000000

void ** plugins;

char * testDir1;
char * testDir2; // these will be used for synchronization.

int get_sync_paths(char *** paths, char *updated){
	char * base = testDir1;
	char * sync_base = testDir2;
	char ** container = malloc(sizeof(char*));
	char * path = malloc(strlen(updated)- strlen(base) + strlen(sync_base) +2);
	container[0] =path;
	printf("%s%s\n",sync_base, updated + strlen(base));
	sprintf(path,"%s%s",sync_base, updated + strlen(base));
	*paths = container;
	return 1;
}

int get_plugin(char * path){
	return 0;
}

int cb(char * path, int mask){
	printf("file %s changed, mask was %.4x\n",path,mask);
	int i;
	char ** sync_path;
	int po = get_plugin(path);
	int(*sync_read)(int,char*,int) 
		= dlsym(plugins[po],"sync_read");
	int (*sync_open_file)(char *,const char*) = dlsym(plugins[po],"sync_open");
	void (*sync_close_file)(unsigned int) = dlsym(plugins[po],"sync_close");
	int file = -1;
	if (mask & (CREATE |CLOSE) )file = sync_open_file(path,"rb");
	int num_paths = get_sync_paths(&sync_path,path);
	for( i = 0; i < num_paths ; i++){
		int pd = get_plugin(sync_path[i]);
		if ((mask & CREATE ) && (mask & ISDIR) && file != -1){
			printf("new dir\n");
			int(*sync_mkdir)(char*) = 
				dlsym(plugins[pd],"sync_mkdir");
			sync_mkdir(sync_path[i]);
		} else if (mask & CLOSE_WRITE && file != -1){
			int (*sync_write)(char*,int,int(*)(int,char*,int))
				= dlsym(plugins[pd],"sync_write");
			printf("wrote %d bytes\n",sync_write(sync_path[i],file,sync_read));
		} else if (mask & DELETE){
			int (*sync_rm)(char*) = dlsym(plugins[pd],"sync_rm");
			printf("deleted file %s, returned %d\n",sync_path[i],sync_rm(sync_path[i]));
		}
	}
	free(sync_path);
	if (file != -1) sync_close_file(file);
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
	testDir1 = calloc(1, 256); //allocate 1 chunk of 256 bytes, 0-initialized
	strcat(testDir1, getenv("HOME"));
	strcat(testDir1, "/Documents"); // for now we need the trailing slash

	testDir2 = calloc(1, 256); //allocate 1 chunk of 256 bytes, 0-initialized
	strcat(testDir2, getenv("HOME"));
	strcat(testDir2, "/synctest"); // TODO: create the destination if it doesn't exist.

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
	}
	pthread_join(t,NULL);
	unloadPlugins(plugins,num_plugins);

	free(testDir1);
	free(testDir2);
}
