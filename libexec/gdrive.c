#include "libgdrive/gdrive_api.h"
#include "../os.h"

#define __GLOBAL_CLIENT_SECRET  "gcyc89d--P9nUb1KagVeV496"
#define __GLOBAL_CLIENT_ID      "969830472849-93kt0dqjevn8jgr3g6erissiocdhk2fo.apps.googleusercontent.com"
#define REFRESH_TOKEN "1/8obRmFxvhhebWSCYckmw_AfUlfTD-ERnwvoro8tMAKI" 

#define PLUGIN_PREFIX "gdrive://"
#define PLUGIN_PREFIX_LEN 9


/* globals */
json_object * cache;

/* structs */
typedef struct{
	const char* title;
	bool is_dir;
} file_info;

/* cache functions*/
void cache_remove(const char* id){
	json_object * files;
	if (json_object_object_get_ex(cache,"files",&files)){
		json_object_object_del(files,id);
	}
}

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

const char * cache_make(){
	json_object * files;
	if (!json_object_object_get_ex(cache,"files",&files)){
		files = json_object_new_object();
		json_object_object_add(cache,"files",files);
	}
	// cludge for now
	return 0;
}

file_info * cache_lookup(const char * id){
	json_object * files;
	if (json_object_object_get_ex(cache,"files",&files)){
		if (json_object_object_get_ex(files,id,&files)){
			file_info * fi = malloc(sizeof(file_info));
			fi->title = JSON_GET_STRING(files,"title");
			fi->is_dir = JSON_GET_BOOL(files,"is_dir",false);	
			return fi;
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

const char * cache_get_id(const char * path){
	json_object * paths;
	json_object * file;
	if (!json_object_object_get_ex(cache,"paths",&paths)){
		paths = json_object_new_object();
		json_object_object_add(cache,"paths",paths);
	}
	if (json_object_object_get_ex(paths,path,&file)){
		return json_object_get_string(file);
	}
	// it wasn't in the cache, so get it and add it.
	return NULL;
}

char * login(){
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
	if (token[len-1] == '\n') token[len-1] == '\0';
	json_object * resp = gdrive_authorize_token(
		token,
		__GLOBAL_CLIENT_ID,
		__GLOBAL_CLIENT_SECRET
	);
	puts(json_object_to_json_string_ext(resp,JSON_C_TO_STRING_PRETTY));
	char * access_token = strdup(JSON_GET_STRING(resp,"access_token"));
	json_object_object_add(cache,"auth",resp);
	json_object_put(resp);
	json_object_to_file("cache.json",cache);
	return access_token;
}

void reauth(){
	json_object * auth;
	char * key;
	if (json_object_object_get_ex(cache,"auth",&auth)){
		char * refresh_token = strdup(JSON_GET_STRING(auth,"refresh_token"));
		if (refresh_token == NULL){
			key = login();
		} else {
			key=gdrive_refresh_token(refresh_token);
                        free(refresh_token);
                        json_object_object_add(
				auth,
				"access_token",
				json_object_new_string(key)
			);
                        json_object_to_file("cache.json",cache);
		}	
	} else {
		key = login();
	}
	gdrive_access_token(key);
}

bool check_error(json_object* obj){
	printf("check_error\n");
	if (json_object_object_get_ex(obj,"error",NULL)){
		printf("doing reauth\n");
		reauth();
		return true;
	}
	return false;
}

/** char * init()
  * returns plugin prefix (eg. “dropbox://”) 
  * performs any startup tasks such as authentication 
  */
char * init(){
	const char * key;
	cache = json_object_from_file("cache.json");
	if (cache == NULL) cache = json_object_new_object();
        curl_global_init(CURL_GLOBAL_DEFAULT);
	json_object * auth;
	if (!json_object_object_get_ex(cache,"auth",&auth) ){
		key = login();
		json_object_object_get_ex(cache,"auth",&auth);
	} else {
		key = JSON_GET_STRING(auth,"access_token");
	}
	if (key == NULL){
		char * refresh_token = strdup(JSON_GET_STRING(auth,"refresh_token"));
		if (refresh_token == NULL) {
			key = login();
		} else {
			key=gdrive_refresh_token(refresh_token);
			free(refresh_token);
			json_object_object_add(auth,"access_token",json_object_new_string(key));
			json_object_to_file("cache.json",cache);
		}
	}
        gdrive_init(key,cache);
	return PLUGIN_PREFIX;
}

/** void sync_unload()
  * performs any shutdown tasks
  */

void sync_unload(){
	curl_global_cleanup();
	gdrive_cleanup();
	json_object_to_file("cache.json",cache);
	json_object_put(cache);
}

void get_updates(int (*cb)(const char*,int)){
	const char * last_change;
        char * next_page_token = NULL;
	json_object* lc;
	if (!json_object_object_get_ex(cache,"largest_change",&lc)){
		last_change = cache_make();
	} else {
		last_change = json_object_get_string(lc);
	}
	if (last_change[0]=='0') last_change = NULL;
        do {
		json_object * changes;
		do {
                	changes = gdrive_get_changes(next_page_token,last_change,1000);
		} while(check_error(changes));

                free(next_page_token);
                const char * temp = JSON_GET_STRING(changes,"nextPageToken");
                if (temp != NULL) next_page_token = strdup(temp);
                int i;
		{
			printf("id=%s\n",json_object_to_json_string(changes));
			long temp = JSON_GET_INT64(changes,"largestChangeId");
			printf("largestchangeID = %d\n",temp);
			char  largestChangeId[128];
			sprintf(largestChangeId,"%d",temp+1);
			json_object_object_add(cache,"largest_change",json_object_new_string(largestChangeId));
			json_object_to_file("cache.json",cache);
		}
                json_object * items;
                if (json_object_object_get_ex(changes, "items",&items)){
		for ( i = 0; i < json_object_array_length(items);i++){
			json_object * change = json_object_array_get_idx(items,i);
			const char * id    = JSON_GET_STRING(change,"fileId");
			if (
				!JSON_GET_BOOL(change,"deleted",false) &&
				json_object_object_get_ex(change,"file",&change)
			){
				const char * title = 
					JSON_GET_STRING(change,"title");
				const char * modified = 
					JSON_GET_STRING(change,"modifiedDate");
				bool is_create = strcmp(
					JSON_GET_STRING(change,"createdDate"),
					modified
				) == 0;
				bool is_dir =  strcmp(
					JSON_GET_STRING(change,"mimeType"),
					"application/vnd.google-apps.folder"
				) == 0;
				int mask;
				if (is_dir) 	mask |= S_ISDIR;
				if (is_create) 	mask |= S_CREATE;
				cb(title,mask);
				printf("file id: '%s', title: '%s%s' %s on %s\n",
					id,
					title,
					is_dir?"/":"",
					is_create?"created":"modified",
					modified
				);
			} else {
				file_info * file= cache_lookup(id);
				int mask;
				if (file != NULL) {
					mask |= S_DELETE;
					if (file->is_dir)mask|=S_ISDIR;
					cb(file->title, mask);
				}
				free(file);
				printf("file id: '%s, deleted\n",id);
				cache_remove(id);
			}
		}
                }
                json_object_put(changes);
        } while (next_page_token != NULL);
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
	char * id = strdup(cache_get_id(path));
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

