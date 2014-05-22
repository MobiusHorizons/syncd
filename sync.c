#include "os.h"
typedef struct {
	Library ptr;
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
	char * base = testDir1; // "dropbox://" "testfolder/myfolder1/test.txt"
	char * sync_base = testDir2; // "fs:///home/paul/DrobBox" "/test.txt"
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
	if (mask & S_MOVED_FROM){
		if (moved_from != NULL) 
			moved_from = free_all(moved_from,num_moved_from);
		num_moved_from = get_sync_paths(&moved_from,path);
	}
	printf("file %s changed, mask was %.4x\n",path,mask);
	int i;
	char ** sync_path;
	int po = get_plugin(path);
	S_OPEN_FILE  sync_open_file = (S_OPEN_FILE)dlsym(plugins[po].ptr,"sync_open");
//	S_CLOSE_FILE sync_close_file = (S_CLOSE_FILE) dlsym(plugins[po].ptr,"sync_close");
	int num_paths = get_sync_paths(&sync_path,path);
	printf(" errno = %d, and num_paths = %d\n",errno,num_paths);
	for( i = 0; i < num_paths ; i++){
		int pd = get_plugin(sync_path[i]);
/*		if (file != NULL){
			FILE * (*of)(char *,const char*) = dlsym(plugins[pd].ptr,"sync_open");
	        	void (*cf)(FILE*) = dlsym(plugins[pd].ptr,"sync_close");
			FILE * tf = of(sync_path[i],"rb");
			if (tf == NULL){
				mask |= S_CLOSE_WRITE;
				cf(tf);
			}
		}*/
	
		if ((mask & S_CREATE ) && (mask & S_ISDIR)){
			printf("new dir\n");
			S_MKDIR sync_mkdir = (S_MKDIR) dlsym(plugins[pd].ptr,"sync_mkdir");
			sync_mkdir(sync_path[i]);
		} else  
		if (mask & S_CLOSE_WRITE ){
			printf("trying to open file\n");
			FILE * file = sync_open_file(path,"rb");
			printf("file opened, file = %p\n",file);
			printf("writing file ...\n");
			if (file !=NULL){
				S_WRITE sync_write = (S_WRITE) dlsym(plugins[pd].ptr,"sync_write");
				printf("starting write\n");
				printf("wrote %d bytes\n",sync_write(sync_path[i],file));
				fclose(file);
			}
		} else
		if (mask & S_DELETE){
//			int (*sync_rm)(char*) 
			S_RM sync_rm = (S_RM) dlsym(plugins[pd].ptr,"sync_rm");
			printf("deleted file %s, returned %d\n",sync_path[i],sync_rm(sync_path[i]));
		} 
		if (mask & S_MOVED_TO){
//			int (*sync_mv)(char*,char*) 
			S_MV sync_mv = (S_MV) dlsym(plugins[pd].ptr,"sync_mv");
			printf("moved file %s to %s, returned %d\n",moved_from[i],sync_path[i],sync_mv(moved_from[i],sync_path[i]));
		}
	}
	if (mask & S_MOVED_TO) moved_from = free_all(moved_from,num_moved_from);
	sync_path = free_all(sync_path,num_paths);
	return 0;
}

/* //these were used for pthread stuff
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
*/

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
#if defined(UNIX)
      			if(ep->d_type == DT_REG){
#endif
				char * filename = malloc(strlen(ep->d_name) + strlen("plugins/")+1);
				sprintf(filename,"plugins/%s",ep->d_name);
				p.ptr = dlopen(filename, RTLD_LAZY);
				if (p.ptr != NULL){
					plugins = realloc(plugins,(num_plugins+1) * sizeof(Plugin) );
					S_INIT init = (S_INIT) dlsym(p.ptr,"init");
					p.prefix = init();
					plugins[num_plugins] = p;
					num_plugins ++ ;
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

void unloadPlugins(Plugin *plugins, int num){
	int i;
	for (i = 0; i < num; i++){
		S_UNLOAD sync_unload = (S_UNLOAD) dlsym(plugins[i].ptr,"sync_unload");
		if (sync_unload != NULL) sync_unload();
		dlclose(plugins[i].ptr);
	}
	free(plugins);
}

void add_watch(char* path){
	int p = get_plugin(path);
	printf("plugin for '%s' is %d\n",path,p);
	if (p != -1){
		S_WATCH_DIR watch_dir = (S_WATCH_DIR) dlsym(plugins[p].ptr,"watch_dir");
		watch_dir(path);
	}
}

int main(int argc, char** argv){
	testDir2 = malloc(256);//alloca(256); //allocate 256 bytes for filename
	testDir1 = malloc(256);//alloca(256); //allocate 256 bytes for filename
	strcpy(testDir1, "dropbox:///");
	strcpy(testDir2, "fs://"); // 2 is dest
#if defined(WIN32)
	strcat(testDir2, "/home/paul/DrDocx");
#else
	strcat(testDir2, getenv("HOME"));// we won't asume the first character is '\0'
	strcat(testDir2, "/cmDropBox/");
#endif
//	pthread_t *t;
//	args_t args;
	printf("t1 = %s, t2 = %s\n",testDir1,testDir2);
	num_plugins = loadPlugins(&plugins);
	printf("got plugins\n");
	int i;
//        t = (pthread_t *)malloc(sizeof(pthread_t) * num_plugins);
	add_watch(testDir1);
//	add_watch(testDir2);
	for ( i = 0; i < num_plugins; i++){
		//void (*listen)( int(*)(char*,int)) = dlsym(plugins[i].ptr,"listen");
		S_LISTEN listen =(S_LISTEN) dlsym(plugins[i].ptr,"sync_listen");
		int pid = fork();
		if (pid == 0){ // child
			listen(cb);
			exit(0);
		}	
//		args.fun = dlsym(plugins[i].ptr,"listen");
//		args.args = cb;
//		pthread_create(&t[i],NULL,(void*) &runner,(void*)&args);
	}
//	for ( i = 0; i < num_plugins; i++){
//		pthread_join(t[i],NULL);
//	}
	unloadPlugins(plugins,num_plugins);
}
