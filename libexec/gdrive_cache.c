#define  _XOPEN_SOURCE 500
#include "gdrive_cache.h"
#include <libgen.h>
#include "../libgdrive/gdrive_api.h"
#include "../src/os.h"
#include <sys/time.h>
#include <string.h>


#define PLUGIN_PREFIX "gdrive://"
#define PLUGIN_PREFIX_LEN 9

utilities utils;
bool check_error(json_object *);

void gdrive_cache_init(utilities u){
    utils = u;
}

char * safe_strdup(const char * value){
    if (value == NULL) return NULL;
    return strdup(value);
}

json_object * get_metadata(const char* id, const char* path){
	/* This function should call ghe gdrive metadata function
	 * or pull from cache. It should update the cache with values that are new.
	 * (ie if path is NULL set it)
	 ****/

	char * id_from_path = NULL;
	char * path_from_id = NULL;

	/* If ID is null, we are looking for a file by it's path */
	if (id == NULL){
		if (path == NULL) return NULL;
		id = id_from_path = get_id(path);
	}

	json_object * file = utils.getFileCache(PLUGIN_PREFIX, id);
	if (file != NULL){
        free(id_from_path);
        return file;
    }

	/* Not in any cache, now we have to get it from google */
	file = update_metadata(id, NULL);

	/* use the supplied path if available else calculate path */
	if (path == NULL) path = path_from_id = get_path(id);

    if (path == NULL){
        free(id_from_path);
        return NULL;// this means it was deleted
    }

	json_add_string(file, "path", path);

	utils.addCache(PLUGIN_PREFIX, path, json_object_get(file));
	utils.addCache(PLUGIN_PREFIX, id  , file);

	json_object_put(file);

	free(id_from_path);
	return file;
}


json_object * update_metadata( const char * id, json_object * gdrive_meta){
    bool free_metadata = false;
	while(check_error(gdrive_meta)) {
        gdrive_meta = gdrive_get_metadata(id);
        free_metadata = true;
        json_object * error;
        if(json_object_object_get_ex(gdrive_meta, "error", &error)){
            if (json_get_int(error, "code", 0) == 404){
                // this is deleted, give a deleted object instead;
                json_object * file = json_object_new_object();
                json_add_bool(file, "deleted", true);
                if (gdrive_meta != NULL) json_object_put(gdrive_meta);
                return file;
            }
        }
	} 

	json_object * file = json_object_new_object();
	bool is_dir =  strcmp(
		json_get_string(gdrive_meta,"mimeType"),
		"application/vnd.google-apps.folder"
	) == 0;

  // get the modified time
  const char * modified_s = json_get_string(gdrive_meta, "modifiedDate");
  if (modified_s != NULL){
    char * mtime = strdup(modified_s);
  	time_t modified = time(0);
    struct tm mod;
    memset(&mod, 0, sizeof(struct tm));
    // time in format 2015-02-12T17:40:31.492Z
  	strptime(mtime,"%Y-%m-%dT%H:%M:%SZ",&mod);
    free(mtime);
    modified = mktime(&mod);
    json_add_int(file, "modified", (long long)modified);
  }
	json_add_bool(file, "is_dir", is_dir);
	json_add_string(file,"title", json_get_string(gdrive_meta, "title"));
	json_add_string(file,"id", id);
	json_add_int(file, "version", 0);
//  if (json_object_object_get_ex(gdrive_meta, "downloadUrl",NULL)){
//    json_add_string(file,"downloadUrl", json_get_string(gdrive_meta,"downloadUrl"));
//  }


	json_object * parents;
	if ( json_object_object_get_ex(gdrive_meta, "parents", &parents)
		   && json_object_array_length(parents) > 0									 ){
				const char * parent_id = json_get_string(json_object_array_get_idx(parents, 0), "id");
				json_add_string(file, "parentID", parent_id);
	} else {
		if (is_dir) json_add_bool(file, "is_root", true);
	}
	//printf("google drive: %s\n", json_object_to_json_string_ext(gdrive_meta, JSON_C_TO_STRING_PRETTY));
	//printf("local       : %s\n", json_object_to_json_string_ext(file, JSON_C_TO_STRING_PRETTY));
  if (free_metadata) json_object_put(gdrive_meta);
  return file;
}

char * get_path(const char * id){
    bool free_metadata = false;
	if (id == NULL) return strdup("/");
	if ( strcmp(id, "root") == 0 ) return strdup("/");

	json_object * file = utils.getFileCache(PLUGIN_PREFIX, id);
	if (file == NULL) {
	    file = update_metadata(id, NULL);
        utils.addCache(PLUGIN_PREFIX, id, file);
        free_metadata = true;
    }
    // #DEBUG
    //printf("file: \n%s\n", json_object_to_json_string_ext(file, JSON_C_TO_STRING_PRETTY));
    if (json_get_bool(file, "deleted", false)) return NULL;
    if (!json_object_object_get_ex(file, "path", NULL)){ 
        if (json_get_bool(file, "is_root", false)) return strdup("/");
        char * parent_id = safe_strdup(json_get_string(file, "parentID"));
        char * title = safe_strdup(json_get_string(file, "title"));
        if (title == NULL){ return NULL ;}
        bool is_dir = json_get_bool(file, "is_dir", false);
        char * path = get_path(parent_id);

        free(parent_id);
        
        if (path == NULL) return NULL; // deleted

        path = (char*) realloc (path, strlen(path) + strlen(title) + 1 + is_dir);
        strcat(path, title);
        if (is_dir) strcat(path, "/");
        free(title);
        
        // cache this data.
        get_metadata(id, path);
        if (free_metadata) json_object_put(file);
        return path;
    }
    if (free_metadata) json_object_put(file);
    return strdup(json_get_string(file, "path"));
}

char * get_child_id(char * parentId, char * fname){
	char * id = NULL;
	char * query = (char*)malloc(strlen(fname) + strlen("title=''") + 1);
	sprintf(query,"title='%s'",fname);
	json_object * dir_info = gdrive_folder_list(parentId, NULL, query);
	json_object * files;
	if (json_object_object_get_ex(dir_info, "items", &files)){
		json_object * file = json_object_array_get_idx(files,0);
		id = strdup(json_get_string(file, "id"));
	}
	json_object_put(dir_info);
	return id;
}

char * get_id(const char * path){
	if (path == NULL) return NULL;

	if (strcmp(path, "/") == 0){
		return strdup("root");
	}

	json_object * paths = utils.getCache(PLUGIN_PREFIX);
	json_object * file;
	if (json_object_object_get_ex(paths,path,&file)){
		return strdup(json_get_string(file,"id"));
	}
	// it wasn't in the cache, so get it and add it.

	char * local_path = strdup(path);
	char * fname = basename(local_path);
	char * parent = dirname(local_path);
	char * parentId = get_id(parent);
	char * id = get_child_id(parentId, fname);
	get_metadata(id, path); // this caches the metadata.
	free(parentId);
	free(local_path);
	return id;
}

void update_version(const char * id, const char * path){
    json_object * md = get_metadata(id, path);
    if (id == NULL) id = json_get_string(md, "id");
    if (path == NULL) id = json_get_string(md, "path");
    if (md == NULL) return;
    json_add_int(md, "version", json_get_int(md, "version", 0) + 1);
    utils.addCache(PLUGIN_PREFIX, id, json_object_get(md));
    utils.addCache(PLUGIN_PREFIX, path, json_object_get(md));
    json_object_put(md);
}
