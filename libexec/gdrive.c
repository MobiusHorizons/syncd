/* Copyright (c) 2014 Paul Martin & Brian Cole
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#define  _XOPEN_SOURCE 500
#include <libgen.h>
#include <sys/wait.h>
#include "../libgdrive/gdrive_api.h"
#include "../src/os.h"
#include "../src/json_helper.h"


#define PLUGIN_PREFIX "gdrive://"
#define PLUGIN_PREFIX_LEN 9
#include "../src/plugin.h"
#include "gdrive_cache.h"


#define __GLOBAL_CLIENT_SECRET  "gcyc89d--P9nUb1KagVeV496"
#define __GLOBAL_CLIENT_ID      "969830472849-93kt0dqjevn8jgr3g6erissiocdhk2fo.apps.googleusercontent.com"
#define REFRESH_TOKEN "1/8obRmFxvhhebWSCYckmw_AfUlfTD-ERnwvoro8tMAKI"

#if defined(_WIN32)
#define URL_OPEN_CMD "start \"\""
#define SILENT_CMD ""
#elif defined(__APPLE__) && defined(__MACH__)
#define URL_OPEN_CMD "open"
#define SILENT_CMD "2&>/dev/null"
#else
#define URL_OPEN_CMD "sh -c 'xdg-open"
#define SILENT_CMD "' 2&>/dev/null"
#endif

bool check_error(json_object* obj);
char * mkdirP(const char * path);


/* globals */
json_object * local_cache;
utilities global_utils;
utilities local_utils;
utilities utils;
init_args args;

json_object * lc_get(){
    if (local_cache == NULL){
        local_cache = json_object_get(global_utils.getCache(PLUGIN_PREFIX));
    }
    if (local_cache == NULL){
        local_cache = json_object_new_object();
    }
    return local_cache;
}

void lc_addCache(const char * plugin_prefix, const char * fname, json_object * entry){
    json_object * c = lc_get();
    if (fname != NULL && strlen(fname) != 0 && entry != NULL){
        args.log(LOGARGS,"adding cache entry for %s\n", fname);
        //puts(json_object_to_json_string_ext(entry, JSON_C_TO_STRING_PRETTY));
        json_object * cache_entry = json_object_get(entry);
        json_object_object_add(c, fname, cache_entry);
        //json_object_put(cache_entry);
    }
}

json_object * lc_getCache(const char * plugin_prefix){
    return lc_get();
}

json_object * lc_getFileCache(const char * plugin_prefix, const char * fname){
    json_object * pcache = lc_get();
    json_object * out;
    if (json_object_object_get_ex(pcache, fname, &out)){
        return out;
    } else {
        return NULL;
    }
}

int update_cache(const char  * id,
                 const char  * path,
                 json_object * new_metadata){
	json_object * old_metadata = utils.getFileCache(PLUGIN_PREFIX, id);
    bool is_dir = json_get_bool(new_metadata, "is_dir", false);
    if (old_metadata != NULL){
        long long int old_mtime = json_get_int(old_metadata, "modified", 0);
	    long long int new_mtime = json_get_int(new_metadata, "modified", 1);
        if (old_mtime >= new_mtime ){
            json_object_put(new_metadata);
            return 0;
        }
    } else {
        if (id) utils.addCache(PLUGIN_PREFIX, id, json_object_get(new_metadata));
        if (path){
            char * clean_path = normalize_path(path, is_dir);
            utils.addCache(PLUGIN_PREFIX, clean_path, json_object_get(new_metadata));
            free(clean_path);
        }
    }

    // else metadata has changed.
    char * path_from_id = NULL;

    int version = json_get_int(old_metadata, "version", 0) + 1;
    json_add_int(new_metadata, "version", version);

    if (path == NULL) {
        path = path_from_id = get_path(id);
    } else {
        path = path_from_id = normalize_path(path, is_dir);
    }

    if (path == NULL) { // this was deleted
        json_object_put(new_metadata);
        return 0;
    }
    json_add_string(new_metadata, "path", path);

       utils.addCache(PLUGIN_PREFIX, path, json_object_get(new_metadata));
    utils.addCache(PLUGIN_PREFIX, id  , new_metadata);
    json_object_put(new_metadata);
    free(path_from_id);
    return 1;
}

unsigned long upload(const char * path, FILE * file){
    char * id = NULL;
    if (path == NULL) return -1;
    id = get_id(path);
    unsigned long size;
    char * local_path = strdup(path);
    json_object * metadata = json_object_new_object();
    char * fname = strdup(basename(local_path));
    const char * parent = dirname(local_path);
    char * parentID = get_id(parent);
    if (parentID == NULL){
        parentID = mkdirP(parent);
    }

    json_add_string(metadata, "title", fname);
    if (id != NULL){
        json_add_string(metadata, "id", id);
    }
    free(id);

    json_object * parent_obj = json_object_new_object();
    json_add_string(parent_obj, "id", parentID);
    json_object * parents = json_object_new_array();
    json_object_array_add(parents, parent_obj);
    json_object_object_add(metadata, "parents", parents);

    json_object * response = NULL;
    {
        do{
            response = gdrive_put_file(metadata, file);
        } while (check_error(response));
    }
    size = json_get_int(response, "fileSize", 0);
	json_object_put(metadata);
    id = strdup(json_get_string(response, "id"));
    json_object * cache_entry = update_metadata(id, response);
    json_add_string(cache_entry, "path", path);
    utils.updateFileCache(PLUGIN_PREFIX, id, json_object_get(cache_entry));
    utils.updateFileCache(PLUGIN_PREFIX, path, cache_entry);
    json_object_put(response);
    free(id);
    free(parentID);
    free(local_path);
    free(fname);
    return size;
}

char * mkdirP(const char * path){
    if (path == NULL) return NULL;

    char * id = get_id(path);
    if (id != NULL) return id;

    char * local_path = strdup(path);
    json_object * metadata = json_object_new_object();
    char * fname = strdup(basename(local_path));
    const char * parent = dirname(local_path);
    char * parentID = get_id(parent);
    if (parentID == NULL){
        parentID = mkdirP(parent);
    }

    json_add_string(metadata, "title", fname);
    json_add_string(metadata, "mimeType", "application/vnd.google-apps.folder" );
    json_object * parent_obj = json_object_new_object();
    json_add_string(parent_obj, "id", parentID);
    json_object * parents = json_object_new_array();
    json_object_array_add(parents, parent_obj);
    json_object_object_add(metadata, "parents", parents);

    json_object * response = NULL;
    {
        do{
            response = gdrive_new_folder(metadata);
        } while (check_error(response));
    }
    id = strdup(json_get_string(response, "id"));
	json_object_put(metadata);
    json_object * cache_entry = update_metadata(id, response);
    json_add_string(cache_entry, "path", path);
    utils.updateFileCache(PLUGIN_PREFIX, id, json_object_get(cache_entry));
    utils.updateFileCache(PLUGIN_PREFIX, path, cache_entry);
    json_object_put(response);
    free(parentID);
    free(local_path);
    free(fname);
    return id;
}


char * login(){
	json_object * config = utils.getConfig(PLUGIN_PREFIX);
	if (config == NULL){
		config = json_object_new_object();
	}
	char token[128];
	char cmd[512];
	sprintf(cmd , "%s \"%s?client_id=%s&redirect_uri=%s&scope=%s&response_type=code\" %s",
	URL_OPEN_CMD,
	"https://accounts.google.com/o/oauth2/auth",
	__GLOBAL_CLIENT_ID,
	"urn:ietf:wg:oauth:2.0:oob",
	"https://www.googleapis.com/auth/drive",
  SILENT_CMD
	);
  int cmd_ret = system(cmd);
  args.log(LOGARGS, "command returned %d\n", WEXITSTATUS(cmd_ret));
  if(WEXITSTATUS(cmd_ret) != 0){
    // We don't have a browser.
    args.log(LOGARGS,"No browser, so we will put the url in the command line.\n");
    printf("oops, you don't seem to have a browser.\n");
    printf("Please go to %s?client_id=%s&redirect_uri=%s&scope=%s&response_type=code and log in.",
  	"https://accounts.google.com/o/oauth2/auth",
  	__GLOBAL_CLIENT_ID,
  	"urn:ietf:wg:oauth:2.0:oob",
  	"https://www.googleapis.com/auth/drive"
  	);
  }
	printf("Paste code here\n");
	if (fgets(token,128,stdin) == NULL) exit(1);
	int len = strlen(token);
	if (token[len-1] == '\n') token[len-1] = '\0';

	json_object * resp = NULL;
	do {
		resp = gdrive_authorize_token(token, __GLOBAL_CLIENT_ID, __GLOBAL_CLIENT_SECRET);
	} while (resp == NULL || ( resp != NULL && json_object_object_get_ex(resp, "error", NULL)));

	args.log(LOGARGS, "%s\n",json_object_to_json_string_ext(resp,JSON_C_TO_STRING_PRETTY));
	char * access_token = strdup(JSON_GET_STRING(resp,"access_token"));
	json_object_object_add(config,"auth",resp);
	utils.addConfig(PLUGIN_PREFIX, config);
	return access_token;
}

void reauth(){
  	json_object * config = json_object_get(utils.getConfig(PLUGIN_PREFIX));
	json_object * auth;
	char * key;

	if (json_object_object_get_ex(config,"auth",&auth)){
	  	key = strdup(json_get_string(auth, "access_token"));

	  	// If the auth token has changed since what we have been using, just use the new one.
	  	if (strcmp(gdrive_access_token(NULL), key) != 0){
	  	  gdrive_access_token(key);
	  	  free(key);
          json_object_put(config);
	  	  return;
	  	}

		char * refresh_token = strdup(JSON_GET_STRING(auth,"refresh_token"));
		if (refresh_token == NULL){
      free(key);
			key = login();
		} else {
      free(key);
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
    free(key);
    //json_object_put(config);
}

bool check_error(json_object* obj){
    if (obj == NULL) return true;
	if (json_object_object_get_ex(obj,"error",NULL)){
        args.log(LOGARGS,"object has error: %s\n", json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PRETTY));
		args.log(LOGARGS,"doing reauth\n");
		reauth();
		return true;
	}
	return false;
}

/** char * init()
* returns plugin prefix (gdrive://)
* performs any startup tasks such as authentication
*/
const char * init(init_args a){
  args = a;
	global_utils = local_utils = utils = args.utils;
    gdrive_cache_init(args.utils);
    gdrive_cache_set_args(args);
    /* set up local cache utils */
    local_utils.addCache = lc_addCache;
    local_utils.getCache = lc_getCache;
    local_utils.getFileCache = lc_getFileCache;

    curl_global_init(CURL_GLOBAL_DEFAULT);
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
    /* switch to local utils */
    {
        utils = local_utils;
        gdrive_cache_init(utils);
    }
    bool first_sync = false;
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
	if (last_change[0]=='0'){
        first_sync = true;
        last_change = NULL;
    }
	do {
        json_object * changes = NULL;
        do {
            if (changes != NULL) json_object_put(changes);
            changes = gdrive_get_changes(next_page_token,last_change,600,false,!first_sync);
        } while(check_error(changes));
        free(last_change);
        last_change = NULL;

        free(next_page_token);
        next_page_token = NULL;
        const char * temp = json_get_string(changes,"nextPageToken");
        if (temp != NULL) next_page_token = strdup(temp);
        int i;
        {
            //args.log(LOGARGS,"id=%s\n",json_object_to_json_string(changes));
            long temp = json_get_int(changes,"largestChangeId", 0);
            args.log(LOGARGS,"largestchangeID = %ld\n",temp);
            char  largestChangeId[128];
            sprintf(largestChangeId,"%ld",temp+1);
            config = json_object_get(utils.getConfig(PLUGIN_PREFIX));
            json_object_object_add(config,"largest_change",json_object_new_string(largestChangeId));
            utils.addConfig(PLUGIN_PREFIX,config);
        }
        json_object * items;
        if (json_object_object_get_ex(changes, "items",&items)){
            for ( i = 0; i < json_object_array_length(items);i++){
                json_object * change = json_object_array_get_idx(items,i);
                const char * id    = json_get_string(change,"fileId");
                if (json_get_bool(change, "deleted", false)) args.log(LOGARGS,"%s was deleted\n",id);
                if (
                    !json_get_bool(change,"deleted",false) &&
                    json_object_object_get_ex(change,"file",&change)
                    ){
                        if (strcmp(json_get_string(change, "mimeType"), "application/vnd.google-apps.folder") == 0){
                            args.log(LOGARGS,"folder id '%s'\n", id);
                            update_cache(id, NULL, update_metadata(id,change));
                        }
                }
            }

            for ( i = 0; i < json_object_array_length(items);i++){
                json_object * change = json_object_array_get_idx(items,i);
                const char * id    = json_get_string(change,"fileId");
                args.log(LOGARGS,"working on changeid %lld\n",json_get_int(change, "id", 0));
                if (
                    !json_get_bool(change,"deleted",false) &&
                    json_object_object_get_ex(change,"file",&change)
                    ){
                        args.log(LOGARGS,"not deleted\n");
                        bool is_trashed = json_get_bool(change, "explicitlyTrashed", false);
                        args.log(LOGARGS,"is_trashed = %s\n", is_trashed? "true":"false");

                        bool can_sync = json_object_object_get_ex(change,"downloadUrl", NULL);
                        if (!is_trashed && !can_sync){
                            // this will end up as a link, but for now ignore.
                            continue;
                        }

                        if (update_cache(id, NULL, update_metadata(id,change)) == 0 && !is_trashed){
                            continue;
                        }

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

                        if (is_trashed){
                            mask = S_DELETE;
                            update_version(id,path);
                            args.log(LOGARGS,"file '%s' with id '%s' was trashed\n", path, id);
                        }

                        char * fullPath = (char *) malloc(PLUGIN_PREFIX_LEN + strlen(path) + 2);
                        strcpy (fullPath, PLUGIN_PREFIX);
                        strcat (fullPath, path);
                        //cb(fullPath,mask);
                        json_add_int(aggregate_changes,fullPath,mask);
                        free(fullPath);

                        args.log(LOGARGS,"file id: '%s', path: '%s' %s on %s\n",
                        id,
                        path,
                        is_create?"created":"modified",
                        modified
                        );
                        free(path);
                    } else {
                        args.log(LOGARGS,"deleting file with id %s\n", id);
                        json_object * file = get_metadata(id, NULL);
                        int mask = 0;
                        if (file != NULL) {
                            mask |= S_DELETE;
                            if (json_get_bool(file, "is_dir", false)) mask|=S_DIR;
                            const char * path = json_get_string(file, "path");
                            if (path != NULL){
                                args.log(LOGARGS,"file '%s' with id '%s' was found in cache\n", path, id);
                                update_version(id,path);
                                char * fullPath = (char *) malloc(PLUGIN_PREFIX_LEN + strlen(path) + 2);
                                strcpy (fullPath, PLUGIN_PREFIX);
                                strcat (fullPath, path);
                                //cb(fullPath,mask);
                                json_add_int(aggregate_changes,fullPath,mask);
                                free(fullPath);
                            }
                        }
                        args.log(LOGARGS,"file id: '%s, deleted\n",id);
                    }
                }
            }
            json_object_put(changes);
        } while (next_page_token != NULL);
    /* push changes back to shared cache */
    {
        utils = global_utils;
        if (local_cache != NULL){
            utils.updateCache(PLUGIN_PREFIX, json_object_get(local_cache));
            json_object_put(local_cache);
            local_cache = NULL;
        }
    }
    json_object_object_foreach(aggregate_changes,path,mask_j){
        cb(path, json_object_get_int64(mask_j));
    }
    json_object_put(aggregate_changes);

	free(next_page_token);
}

const char * get_prefix(){
	return PLUGIN_PREFIX;
}

/** void sync_listen( int ( * call_back )( const char * path, int type ) )
* listens for changes. calls call_back for each change
* path is the name of the file that changed
* type is mask of the type of change that occurred
* (directory, update, create, delete, move, etc)
*/
void sync_listen( int (*call_back)(const char*path,int type)){
  CATCH_EVENTS
	// cludge because we are polling.
	#define poll_time 30 // poll time in seconds
    time_t poll_start;
    time_t now;
	while(1){
    time(&poll_start);
		get_updates(call_back);
    CLEAN_BREAK
    if (time(&now) - poll_start < poll_time){
		sleep(poll_time - (now - poll_start));
    }
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
	FILE * file;
    json_object * fcache = get_metadata(NULL,path);
    const char * id = json_get_string(fcache, "id");
  	do {
		file = gdrive_get(id);
		if (file == NULL) reauth();
	} while (file == NULL);

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
    int resp = upload(path,fp);
    return resp;
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
