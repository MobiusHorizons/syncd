#ifndef __GDRIVE_CACHE_H__
#define __GDRIVE_CACHE_H__
#include <stdbool.h>
#include <json-c/json.h>
#include "../src/plugin.h"
#include "../src/json_helper.h"

void gdrive_cache_init(utilities u, bool (*check_error)(json_object*));

json_object * get_metadata(char *id, const char *path);
json_object * update_metadata(const char * id);
char * get_id(const char * path);
char * get_path(const char * id);
json_object * update_metadata(const char * id);

#endif
