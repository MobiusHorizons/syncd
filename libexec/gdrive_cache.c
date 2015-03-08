#define  _XOPEN_SOURCE 500
#include "gdrive_cache.h"
#include <libgen.h>
#include "../libgdrive/gdrive_api.h"
#include "../src/os.h"

utilities utils;
bool (*check_error)(json_object *);

gdrive_cache_init(utilities u, bool(*ce)(json_object*)){
  check_error = ce;
  utils = u;
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
	if (file != NULL) return file;

	/* Not in any cache, now we have to get it from google */
	file = update_metadata(id);

	/* use the supplied path if available else calculate path */
	if (path == NULL) path = path_from_id = get_path(id);

	json_add_string(file, "path", path);

	utils.addCache(PLUGIN_PREFIX, path, json_object_get(file));
	utils.addCache(PLUGIN_PREFIX, id  , json_object_get(file));

	json_object_put(file);

	free(id_from_path);
	return file;
}


json_object * update_metadata( const char * id){
	json_object * gdrive_meta;

	do {
			gdrive_meta = gdrive_get_metadata(id);
	} while(check_error(gdrive_meta));

	json_object * file = json_object_new_object();
	bool is_dir =  strcmp(
		json_get_string(gdrive_meta,"mimeType"),
		"application/vnd.google-apps.folder"
	) == 0;
	json_add_bool(file, "is_dir", is_dir);
	json_add_string(file,"title", json_get_string(gdrive_meta, "title"));
	json_add_string(file,"id", id);
	json_add_int(file, "version", 0);

	json_object * parents;
	if ( json_object_object_get_ex(gdrive_meta, "parents", &parents)
		   && json_object_array_length(parents) > 0									 ){
				const char * parent_id = json_get_string(json_object_array_get_idx(parents, 0), "id");
				json_add_string(file, "parentID", parent_id);
	} else {
		if (is_dir) json_add_bool(file, "is_root", true);
	}

	json_add_string(file, "headRevisionId", json_get_string(gdrive_meta, "headRevisionId"));
	//printf("google drive: %s\n", json_object_to_json_string_ext(gdrive_meta, JSON_C_TO_STRING_PRETTY));
	//printf("local       : %s\n", json_object_to_json_string_ext(file, JSON_C_TO_STRING_PRETTY));
  return file;
}

char * get_path(const char * id){
	if (id == NULL) return strdup("/");
	if ( strcmp(id, "root") == 0 ) return strdup("/");

	json_object * file = utils.getFileCache(PLUGIN_PREFIX, id);
	if (file == NULL) {
	    file = update_metadata(id);
			if (json_get_bool(file, "is_root", false)) return strdup("/");

      const char * parent_id = json_get_string(file, "parentID");
      const char * title = json_get_string(file, "title");
			bool is_dir = json_get_bool(file, "is_dir", false);
      char * path = get_path(parent_id);
      path = realloc (path, strlen(path) + strlen(title) + 1 + is_dir );
      strcat(path, title);
			if (is_dir) strcat(path, "/");

      // cache this data.
      get_metadata(id, path);

      return path;
	}
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

