#ifndef __CACHE_H_
#define __CACHE_H_

#include <json-c/json.h>

/* Definition of functions provided by sync
 */


/* Cache management ( syncronization between processes ) */
void cache_init();


json_object * getCache(const char * plugin_prefix);
json_object * getFileCache(const char * plugin_prefix, const char * fname);
json_object * getConfig(const char * plugin_prefix);

void addCache(const char * plugin_prefix, const char * fname, json_object * cache_entry);
void updateFileCache(const char * plugin_prefix, const char * fname, json_object * changes);
void updateCache(const char * plugin_prefix, json_object * config);
void addConfig(const char * plugin_prefix, json_object * config);

typedef json_object* (*GETCACHE)    (const char*);
typedef json_object* (*GETFILECACHE)(const char*, const char*);
typedef json_object* (*GETCONFIG)   (const char*);

typedef void (*ADDCACHE) (const char*, const char*, json_object* );
typedef void (*UPDATEFILECACHE) (const char*, const char*, json_object* );
typedef void (*UPDATECACHE)(const char*, json_object *);
typedef void (*ADDCONFIG)(const char*, json_object *);


typedef struct {
    GETCACHE getCache;
    GETFILECACHE getFileCache;
    GETCONFIG getConfig;
    ADDCACHE addCache;
    UPDATEFILECACHE updateFileCache;
    UPDATECACHE updateCache;
    ADDCONFIG addConfig;
} utilities;

utilities get_utility_functions();


#endif
