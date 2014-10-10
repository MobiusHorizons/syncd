#include "os.h"
#include "cache.h"
#include <unistd.h>
#include "json_helper.h"

#define DEBUG
typedef struct {
	Library ptr;
	const char * prefix;
} Plugin;

Plugin * plugins;
int num_plugins;

json_object * rules;


json_object * getCacheDetails(int pnum, const char * path) {
        const char * plugin_prefix = plugins[pnum].prefix;                                 
        json_object * foc = getFileCache(plugin_prefix,path +strlen(plugin_prefix)); 
        json_object * details = json_object_new_object();                            
        json_copy(&details,"size",foc, json_object_new_int64(0));
        json_copy(&details, "modified", foc,json_object_new_int64(0));
        return details;
}

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

int get_sync_paths(char *** paths, const char *updated){
    json_object * r_paths = json_object_new_array();
    int r_paths_length = 0;
    int i;
    json_object_object_foreach(rules,base,targets){
        int array_length = json_object_array_length(targets);
        for(i = 0; i < array_length; i++){
            const char * sync_base = json_object_get_string(json_object_array_get_idx(targets,i));
            if (strncmp(base,updated,strlen(base))==0){
                char * path = malloc(strlen(updated)- strlen(base) + strlen(sync_base) + 2);
                printf("%s%s",sync_base, updated + strlen(base));
                sprintf(path,"%s%s",sync_base, updated + strlen(base));
                json_object_array_put_idx(r_paths,r_paths_length++,json_object_new_string(path));
            }
        }
    }   
    char ** container = malloc(r_paths_length * sizeof(char*));
    for(i = 0; i < r_paths_length; i++){
        container[i] = strdup(
            json_object_get_string(
                json_object_array_get_idx(r_paths,i)
            )
        );
    }
    *paths = container;
    json_object_put(r_paths);
    return r_paths_length;
}

int get_plugin(const char * path){
	int i;
	for (i = 0; i < num_plugins; i++){
		const char * pfx = plugins[i].prefix;
		if (strncmp(pfx,path,strlen(pfx)) == 0) return i;
	}
	return -1; // error
}

int cb(const char * path, int mask){
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
    json_object * orig_cache = json_object_get(
        getFileCache(plugins[po].prefix, path + strlen(plugins[po].prefix))
    );

    if (orig_cache == NULL) orig_cache = json_object_new_object();
	S_OPEN_FILE  sync_open_file = (S_OPEN_FILE)dlsym(plugins[po].ptr,"sync_open");
	int num_paths = get_sync_paths(&sync_path,path);
	printf(" errno = %d, and num_paths = %d\n",errno,num_paths);
	for( i = 0; i < num_paths ; i++){
		int pd = get_plugin(sync_path[i]);
        char * dest_prefix = plugins[pd].prefix;
        json_object * dest_cache = 
            getFileCache(dest_prefix, sync_path[i] + strlen(dest_prefix));
        if (dest_cache == NULL) dest_cache = json_object_new_object();
//        json_object * dest_detail = getCacheDetails(pd,sync_path[i]);
/*        if (
                json_get_int(orig_detail, "size",1) == json_get_int(dest_detail, "size",2) &&
                json_get_int(orig_detail, "modified",1)==json_get_int(dest_detail, "modified",2)
           ) {*/
        long long int orig_ver = json_get_int(orig_cache, "version",0);
        printf("original version = %d;",orig_ver);
        if ( json_get_int(dest_cache, "version", -1) >= orig_ver){
            printf("destination version = %d\n",json_get_int(dest_cache, "version",-1));
            // this file is already in sync
            continue;
        } else {
            //updateFileCache(plugins[pd].prefix, sync_path[i] + strlen(plugins[pd].prefix),orig_detail);
            json_object_object_add(dest_cache, "next_version", json_object_new_int64(orig_ver));
            addCache(dest_prefix, sync_path[i] + strlen(dest_prefix),dest_cache);
        }
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
			FILE * file = sync_open_file(path);
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
			S_RM sync_rm = (S_RM) dlsym(plugins[pd].ptr,"sync_rm");
			printf("deleted file %s, returned %d\n",sync_path[i],sync_rm(sync_path[i]));
		} 
		if (mask & S_MOVED_TO){
			S_MV sync_mv = (S_MV) dlsym(plugins[pd].ptr,"sync_mv");
			printf("moved file %s to %s, returned %d\n",moved_from[i],sync_path[i],sync_mv(moved_from[i],sync_path[i]));
		}
	}
	if (mask & S_MOVED_TO) moved_from = free_all(moved_from,num_moved_from);
	sync_path = free_all(sync_path,num_paths);
    json_object_put(orig_cache);
	return 0;
}

int loadPlugins(Plugin **return_plugins){
	Plugin * plugins = NULL;
	Plugin p;
    utilities u = get_utilities();
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
					p.prefix = init(u);
					printf ("prefix for plugin %d is '%s'\n",num_plugins,p.prefix);
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
    int plugin_to_run = -1;
    { 
        int opt;
        while (( opt = getopt(argc,argv,"p:")) != -1){
            printf("%c\n", opt);
            if (opt == 'p'){
                plugin_to_run = atoi(optarg);
            }
        }
    }
    printf ("plugin_to_run = %d\n",plugin_to_run);
    rules = json_object_from_file("rules.json");
	num_plugins = loadPlugins(&plugins);
	printf("got plugins\n");
	int i;
    json_object_object_foreach(rules,dir,val){
        printf("dir=%s\n",dir);
        add_watch(dir);
    }
    // init shared memory
    cache_init();
    if (plugin_to_run != -1){
        S_LISTEN listen =(S_LISTEN) dlsym(plugins[plugin_to_run].ptr,"sync_listen");
        listen(cb);
    } else 	for ( i = 0; i < num_plugins; i++){
		S_LISTEN listen =(S_LISTEN) dlsym(plugins[i].ptr,"sync_listen");
		int pid = fork();
        if (pid != 0) printf ("forked child %d\n sleeping 15",pid);
		if (pid == 0){ // child
#ifdef DEBUG
            sleep(15);
#endif
			listen(cb);
			exit(0);
		}	
	}
	unloadPlugins(plugins,num_plugins);
}
