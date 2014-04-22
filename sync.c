#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <alloca.h>
#include <errno.h>




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

typedef struct {
	void * ptr;
	const char * prefix;
} Plugin;

Plugin * plugins;
int num_plugins;

char * testDir1;
char * testDir2; // these will be used for synchronization.

char ** free_all(char ** array, int length){
	int i;
	if (array == NULL) length = 0;
	for (i = 0; i < length; i++){
		free(array[i]);
	}
	free(array);
	array = NULL;
	return NULL;
}

int get_sync_paths(char *** paths, char *updated){
	char * base = testDir1;
	char * sync_base = testDir2;
	char ** container = malloc(sizeof(char*));
	char * path = malloc(strlen(updated)- strlen(base) + strlen(sync_base) +2);
	container[0] =path;
	if (strncmp(base,updated,strlen(base))==0){ // if it matches the base
		printf("%s%s\n",sync_base, updated + strlen(base));
		sprintf(path,"%s%s",sync_base, updated + strlen(base));
		*paths = container;
		return 1;
	}else {
		*paths = NULL;
		return 0;
	}
}

int get_plugin(char * path){
	int i;
	for (i = 0; i < num_plugins; i++){
		const char * pfx = plugins[i].prefix;
		if (strncmp(pfx,path,strlen(pfx)) == 0) return i;
	}
	return -1; // error
}

int cb(char * path, int mask){
	static char ** moved_from;
	static int num_moved_from;
	if (mask & MOVED_FROM){
		if (moved_from != NULL) 
			moved_from = free_all(moved_from,num_moved_from);
		num_moved_from = get_sync_paths(&moved_from,path);
	}
	printf("file %s changed, mask was %.4x\n",path,mask);
	int i;
	char ** sync_path;
	int po = get_plugin(path);
	FILE* (*sync_open_file)(char *,const char*) = dlsym(plugins[po].ptr,"sync_open");
	void (*sync_close_file)(FILE*) = dlsym(plugins[po].ptr,"sync_close");
	printf("trying to open file\n");
	FILE * file = sync_open_file(path,"rb");
	printf("file opened, file = %p\n",file);
	int num_paths = get_sync_paths(&sync_path,path);
	printf(" errno = %d, and num_paths = %d\n",errno,num_paths);
	for( i = 0; i < num_paths ; i++){
		int pd = get_plugin(sync_path[i]);
/*		if (file != NULL){
			FILE * (*of)(char *,const char*) = dlsym(plugins[pd].ptr,"sync_open");
	        	void (*cf)(FILE*) = dlsym(plugins[pd].ptr,"sync_close");
			FILE * tf = of(sync_path[i],"rb");
			if (tf == NULL){
				mask |= CLOSE_WRITE;
				cf(tf);
			}
		}*/
	
		if ((mask & CREATE ) && (mask & ISDIR) && file != NULL){
			printf("new dir\n");
			int(*sync_mkdir)(char*) = 
				dlsym(plugins[pd].ptr,"sync_mkdir");
			sync_mkdir(sync_path[i]);
		} else  
		if (mask & CLOSE_WRITE && file != NULL){
			printf("writing file ...\n");
			int (*sync_write)(char*,FILE *)
				= dlsym(plugins[pd].ptr,"sync_write");
			printf("starting write\n");
			printf("wrote %d bytes\n",sync_write(sync_path[i],file));
		} else
		if (mask & DELETE){
			int (*sync_rm)(char*) = dlsym(plugins[pd].ptr,"sync_rm");
			printf("deleted file %s, returned %d\n",sync_path[i],sync_rm(sync_path[i]));
		} 
		if (mask & MOVED_TO){
			int (*sync_mv)(char*,char*) = dlsym(plugins[pd].ptr,"sync_mv");
			printf("moved file %s to %s, returned %d\n",moved_from[i],sync_path[i],sync_mv(moved_from[i],sync_path[i]));
		}
	}
	if (mask & MOVED_TO) moved_from = free_all(moved_from,num_moved_from);
	sync_path = free_all(sync_path,num_paths);
	if (file != NULL) sync_close_file(file);
	return 0;
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

int loadPlugins(Plugin **return_plugins){
	Plugin * plugins = NULL;
	Plugin p;
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
				fflush(stdout);
				p.ptr = dlopen(filename, RTLD_LAZY);
				if (p.ptr != NULL){
					plugins = realloc(plugins,(num_plugins+1) * sizeof(Plugin) );
					char * (*init)() = dlsym(p.ptr,"init");
					p.prefix = init();
					plugins[num_plugins] = p;
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

void unloadPlugins(Plugin *plugins, int num){
	int i;
	for (i = 0; i < num; i++){
		dlclose(plugins[i].ptr);
	}
	free(plugins);
}

void add_watch(char* path){
	int p = get_plugin(path);
	printf("plugin for '%s' is %d\n",path,p);
	if (p != -1){
		void (*watch_dir)(char*) = dlsym(plugins[p].ptr,"watch_dir");
		watch_dir(path);
	}
}

int main(int argc, char** argv){
	testDir1 = alloca(256); //allocate 256 bytes for filename
//	strcpy(testDir1, "fs://");
	strcat(testDir1, getenv("HOME"));// we won't asume the first character is '\0'
	strcpy(testDir1, "fs:///home/paul/Documents");

	testDir2 = alloca(256); //allocate 256 bytes for filename
//	strcpy(testDir2, "fs://");
//	strcat(testDir2, getenv("HOME"));
	strcpy(testDir2, "dropbox:///testdier-1235asdf"); // TODO: create the destination if it doesn't exist.

	pthread_t *t;
	args_t args;
	printf("t1 = %s, t2 = %s\n",testDir1,testDir2);
	num_plugins = loadPlugins(&plugins);
	printf("got plugins\n");
	int i;
        t = (pthread_t *)malloc(sizeof(pthread_t) * num_plugins);
	add_watch(testDir1);
	add_watch(testDir2);
	for ( i = 0; i < num_plugins; i++){
		printf("here\n");
		args.fun = dlsym(plugins[i].ptr,"listen");
		printf("here\n");
		args.args = cb;
		printf("here\n");
		pthread_create(&t[i],NULL,(void*) &runner,(void*)&args);
		printf("here\n");
	}
	for ( i = 0; i < num_plugins; i++){
		pthread_join(t[i],NULL);
	}
	unloadPlugins(plugins,num_plugins);
	free(t);
}
