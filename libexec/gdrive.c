#define  _XOPEN_SOURCE 500
#include <libgen.h>
#include "../libgdrive/gdrive_api.h"
#include "../src/os.h"
#include "../src/json_helper.h"
#include "../src/plugin.h"
#include "gdrive_cache.h"


#define __GLOBAL_CLIENT_SECRET  "gcyc89d--P9nUb1KagVeV496"
#define __GLOBAL_CLIENT_ID      "969830472849-93kt0dqjevn8jgr3g6erissiocdhk2fo.apps.googleusercontent.com"
#define REFRESH_TOKEN "1/8obRmFxvhhebWSCYckmw_AfUlfTD-ERnwvoro8tMAKI"

#define PLUGIN_PREFIX "gdrive://"
#define PLUGIN_PREFIX_LEN 9


bool check_error(json_object* obj);


/* globals */
//json_object * cache;
//json_object * config;
utilities utils;

/* structs */
/*typedef struct{
	const char* title;
	bool is_dir;
} file_info;
*/
/* cache functions*/
/*void cache_remove(const char* id){
	json_object * files;
	if (json_object_object_get_ex(cache,"files",&files)){
		json_object_object_del(files,id);
	}
}
*/

int update_cache(const char  * id,
                 const char  * path,
                 json_object * new_metadata){
	json_object * old_metadata = utils.getFileCache(PLUGIN_PREFIX, id);
    if (old_metadata != NULL){
        long long int old_mtime = json_get_int(old_metadata, "modified", 0);
	    long long int new_mtime = json_get_int(new_metadata, "modified", 1);
        if (old_mtime >= new_mtime ) return 0;
    } else {
//        if (id) utils.addCache(PLUGIN_PREFIX, id, json_object_get(new_metadata));
//        if (path) utils.addCache(PLUGIN_PREFIX, path, json_object_get(new_metadata));
    }

    // else metadata has changed.
    char * path_from_id = NULL;

    int version = json_get_int(old_metadata, "version", 0);
    version++;
    json_object_object_del(new_metadata, "version");
    json_add_int(new_metadata, "version", version);

    if (path == NULL) path = path_from_id = get_path(id);
    if (path == NULL) { // this was deleted
        return 0;
    }
    json_add_string(new_metadata, "path", path);

    utils.addCache(PLUGIN_PREFIX, path, json_object_get(new_metadata));
    utils.addCache(PLUGIN_PREFIX, id  , json_object_get(new_metadata));
    json_object_put(new_metadata);

    return 1;
}



/*
void cache_add(const char *id,file_info fi){
	json_object * files;
	json_object * file;
	if (!json_object_object_get_ex(cache,"files",&files)){
		files = json_object_new_object();
		json_object_object_add(cache,"files",files);
	}
	json_object_object_add(file,"title",json_object_new_string(fi.title));
	json_object_object_add(file,"is_dir",json_object_new_boolean(fi.is_dir));
	json_object_object_add(files,id,file);
}

file_info * cache_lookup(const char * id){
	json_object * files = utils.getCache(PLUGIN_PREFIX);
	if (json_object_object_get_ex(files,id,&files)){
		file_info * fi = malloc(sizeof(file_info));
		fi->title = JSON_GET_STRING(files,"title");
		fi->is_dir = JSON_GET_BOOL(files,"is_dir",false);
		return fi;
	} else {
		return NULL;
	}
}
 */



char * login(){
	json_object * config = utils.getConfig(PLUGIN_PREFIX);
	if (config == NULL){
		config = json_object_new_object();
	}
	char token[128];
	char cmd[512];
	#ifdef WIN32
	#define URL_OPEN_CMD "start \"\""
	#else
	#define URL_OPEN_CMD "xdg-open"
	#endif
	sprintf(cmd , "%s \"%s?client_id=%s&redirect_uri=%s&scope=%s&response_type=code\"",
	URL_OPEN_CMD,
	"https://accounts.google.com/o/oauth2/auth",
	__GLOBAL_CLIENT_ID,
	"urn:ietf:wg:oauth:2.0:oob",
	"https://www.googleapis.com/auth/drive"
	);
	system(cmd);
	printf("paste code here\n");
	if (fgets(token,128,stdin) == NULL) exit(1);
	int len = strlen(token);
	if (token[len-1] == '\n') token[len-1] = '\0';

	json_object * resp = NULL;
	do {
		resp = gdrive_authorize_token(token, __GLOBAL_CLIENT_ID, __GLOBAL_CLIENT_SECRET);
	} while (resp == NULL || ( resp != NULL && json_object_object_get_ex(resp, "error", NULL)));

	puts(json_object_to_json_string_ext(resp,JSON_C_TO_STRING_PRETTY));
	char * access_token = strdup(JSON_GET_STRING(resp,"access_token"));
	json_object_object_add(config,"auth",resp);
	utils.addConfig(PLUGIN_PREFIX, config);
	return access_token;
}

void reauth(){
  	json_object * config = utils.getConfig(PLUGIN_PREFIX);
	json_object * auth;
	char * key;

	if (json_object_object_get_ex(config,"auth",&auth)){
	  	key = strdup(json_get_string(auth, "access_token"));

	  	// If the auth token has changed since what we have been using, just use the new one.
	  	if (strcmp(gdrive_access_token(NULL), key) != 0){
	  	  gdrive_access_token(key);
	  	  free(key);
	  	  return;
	  	}

		char * refresh_token = strdup(JSON_GET_STRING(auth,"refresh_token"));
		if (refresh_token == NULL){
			key = login();
		} else {
			key = gdrive_refresh_token(refresh_token);
			free(refresh_token);
			json_object_object_add(
				auth,
				"access_token",
				json_object_new_string(key)
			);
		  	utils.addConfig(PLUGIN_PREFIX,config);
		}
	} else {
		key = login();
	}
	gdrive_access_token(key);
}

bool check_error(json_object* obj){
    if (obj == NULL) return true;
	if (json_object_object_get_ex(obj,"error",NULL)){
        printf("object has error: %s\n", json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PRETTY));
		printf("doing reauth\n");
		reauth();
		return true;
	}
	return false;
}

/** char * init()
* returns plugin prefix (gdrive://)
* performs any startup tasks such as authentication
*/
char * init(init_args args){
	utils = args.utils;
    gdrive_cache_init(args.utils, &check_error);
	/*cache = utils.getCache(PLUGIN_PREFIX);
	if (cache == NULL){
		cache = json_object_new_object();
	}*/
	const char * key;
	//if (cache == NULL) cache = json_object_new_object();
	curl_global_init(CURL_GLOBAL_DEFAULT);
	json_object * auth;
	gdrive_init("null",NULL);
	reauth();
	return PLUGIN_PREFIX;
}

/** void sync_unload()
* performs any shutdown tasks
*/

void sync_unload(){
	curl_global_cleanup();
	gdrive_cleanup();
}

void get_updates(int (*cb)(const char*,int)){
	char * last_change;
	char * next_page_token = NULL;
	json_object* lc;
    json_object * aggregate_changes = json_object_new_object();

	json_object* config = utils.getConfig(PLUGIN_PREFIX);
	if (!json_object_object_get_ex(config,"largest_change",&lc)){
		last_change = strdup("0");
	} else {
		last_change = strdup(json_object_get_string(lc));
	}
	if (last_change[0]=='0') last_change = NULL;
	do {
        json_object * changes;
        do {
            changes = gdrive_get_changes(next_page_token,last_change,1000);
        } while(check_error(changes));
        free(last_change);
        last_change = NULL;

        free(next_page_token);
        const char * temp = json_get_string(changes,"nextPageToken");
        if (temp != NULL) next_page_token = strdup(temp);
        int i;
        {
            //printf("id=%s\n",json_object_to_json_string(changes));
            long temp = json_get_int(changes,"largestChangeId", 0);
            printf("largestchangeID = %ld\n",temp);
            char  largestChangeId[128];
            sprintf(largestChangeId,"%ld",temp+1);
            config = utils.getConfig(PLUGIN_PREFIX);
            json_object_object_add(config,"largest_change",json_object_new_string(largestChangeId));
            utils.addConfig(PLUGIN_PREFIX,config);
        }
        json_object * items;
        if (json_object_object_get_ex(changes, "items",&items)){
            for ( i = 0; i < json_object_array_length(items);i++){
                json_object * change = json_object_array_get_idx(items,i);
                const char * id    = json_get_string(change,"fileId");
                if (
                    !json_get_bool(change,"deleted",false) &&
                    json_object_object_get_ex(change,"file",&change)
                    ){
                        bool can_sync = json_object_object_get_ex(change,"downloadUrl", NULL);
                        if (! can_sync){
                            // this will end up as a link, but for now ignore.
                            continue;
                        }
    //					json_object * file = update_metadata(id);
                        if (update_cache(id, NULL, update_metadata(id,json_object_get(change))) == 0) continue;

//                        const char * title = json_get_string(change,"title");
                        const char * modified = json_get_string(change,"modifiedDate");

                        bool is_create = strcmp(
                            json_get_string(change,"createdDate"),
                            modified
                        ) == 0;

                        bool is_dir =  strcmp(
                            json_get_string(change,"mimeType"),
                            "application/vnd.google-apps.folder"
                        ) == 0;


                        char * path = get_path(id);

                        int mask = 0;
                        if (is_dir) 	mask |= S_DIR;
                        if (is_create) 	mask |= S_CREATE;
                        else 			mask |= S_CLOSE_WRITE;
                        char * fullPath = (char *) malloc(PLUGIN_PREFIX_LEN + strlen(path) + 2);
                        strcpy (fullPath, PLUGIN_PREFIX);
                        strcat (fullPath, path);
                        //cb(fullPath,mask);
                        json_add_int(aggregate_changes,fullPath,mask); 
                        free(fullPath);

                        printf("file id: '%s', path: '%s' %s on %s\n",
                        id,
                        path,
                        is_create?"created":"modified",
                        modified
                        );
                        free(path);
                    } else {
                        json_object * file = get_metadata(id, NULL);
                        int mask = 0;
                        if (file != NULL) {
                            mask |= S_DELETE;
                            if (json_get_bool(file, "is_dir", false)) mask|=S_DIR;
                            const char * path = json_get_string(file, "path");
                            char * fullPath = (char *) malloc(PLUGIN_PREFIX_LEN + strlen(path) + 2);
                            strcpy (fullPath, PLUGIN_PREFIX);
                            strcat (fullPath, path);
                            //cb(fullPath,mask);
                            json_add_int(aggregate_changes,fullPath,mask);
                            free(fullPath);
                        }
                        printf("file id: '%s, deleted\n",id);
                    }
                }
            }
            json_object_put(changes);
        } while (next_page_token != NULL);
    json_object_object_foreach(aggregate_changes,path,mask_j){
        cb(path, json_object_get_int64(mask_j));
    }
    json_object_put(aggregate_changes);
	free(next_page_token);
}

/** void sync_listen( int ( * call_back )( const char * path, int type ) )
* listens for changes. calls call_back for each change
* path is the name of the file that changed
* type is mask of the type of change that occurred
* (directory, update, create, delete, move, etc)
*/
void sync_listen( int (*call_back)(const char*path,int type)){
	// cludge because we are polling.
	#define poll_time 30 // poll time in seconds
	while(1){
		get_updates(call_back);
		sleep(poll_time);
	}
}

/** void watch_dir( char * path )
* adds path to be watched for changes the plugin should only report changes
* that occur in watched directories.
*/
void watch_dir(char * path){

}

/** FILE * sync_open( char * path )
* opens path for reading, and returns a FILE handle to this resource.
* this file is not required to be seekable, ie it may be a pipe/stream.
*/

FILE * sync_open(char * path){
	path += PLUGIN_PREFIX_LEN;
	char * id = get_id(path);
	FILE * file;

  	do {
		file = gdrive_get(id);
		if (file == NULL) reauth();
	} while (file == NULL);

	free(id);
	return file;
}

/** int sync_write( char * path, FILE * fp )
* write file. this is required to create the directory if it does not exist.
* path: the destination path for the file
* fp: the file stream to write. This is not necessarily seekable.
* returns the number of bytes written.
* NOTE: this function may need to accept MIME type and file size later.
*/
int sync_write(char * path, FILE * fp){
	path += PLUGIN_PREFIX_LEN;
	json_object * response;
	do{
		response = gdrive_files_put(path,fp);
	}while (check_error(response));
	puts(json_object_to_json_string_ext(response,JSON_C_TO_STRING_PRETTY));
	json_object_put(response);
}

/** int sync_mkdir( char * path )
* create directory. this is required to create all parents
* of the final directory that do not exist.
* this behavior is akin to mkdir -p on *nix
* returns non-zero if error
*/
int sync_mkdir(char * path){
	return 0;
}

/** int sync_rm( char * path )
* remove path whether file or directory. akin to rm -r on linux.
* return non-zero if error
*/
int sync_rm(char * path){
	return 0;
}

/** int sync_mv( char * from, char * to )
* move file from to new location to.
* return non-zero on error
*/
int sync_mv(char * path){
	return 0;
}
