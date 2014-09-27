#include "cache.h"
#include <string.h>
#include <stdio.h>

json_object * cache;
json_object * config;

/* definition of local utility functions */
void update_cache();
void update_config();
void push_config();
void push_cache();

/* public functions */
json_object * getCache(const char * plugin_prefix){
    json_object * pluginCache;
    update_cache(); // force synchronization.
    if (json_object_object_get_ex(cache, plugin_prefix, &pluginCache)){
        return pluginCache;
    } else {
        return NULL;
    }
}


json_object * getFileCache(const char * plugin_prefix,const char * fname){
    json_object * pcache = getCache(plugin_prefix);
    json_object * fcache;
    if (pcache != NULL && json_object_object_get_ex(pcache, fname, &fcache)){
        return fcache;
    } else {
        return NULL;
    }
}

json_object * getConfig(const char * plugin_prefix){
    json_object * pluginConfig;
    update_config(); // force synchronization.
    if (json_object_object_get_ex(config, plugin_prefix, &pluginConfig)){
        return pluginConfig;
    } else {
        return NULL;
    }
}

void addCache(const char * plugin_prefix, const char * fname, json_object * cache_entry){
    json_object * pcache = getCache(plugin_prefix);
    if (pcache == NULL){/* create cache for plugin*/
        pcache = json_object_new_object();
        json_object_object_add(cache, plugin_prefix, pcache);
    }
    if (fname != NULL && strlen(fname) != 0 && cache_entry != NULL){
        json_object_object_add(pcache, fname, cache_entry);
        push_cache();
    }
}

void updateFileCache(const char * plugin_prefix, const char * fname, json_object * changes){
    json_object * fcache = getFileCache(plugin_prefix,fname);
    if (fcache == NULL) {
        fcache = json_object_new_object();
        addCache(plugin_prefix, fname, fcache);
    }
    // add each field in changes
    json_object_object_foreach(changes, key,val){
        json_object_object_add(fcache, key, val);
    }
    push_cache();
}

void addConfig(const char * plugin_prefix, json_object * pconfig){
    update_config();
    json_object_object_add(config, plugin_prefix, pconfig);
    push_config();
}

utilities get_utilities(){
    utilities u;
    u.getCache = getCache;
    u.addCache = addCache;
    u.updateFileCache = updateFileCache;
    u.getFileCache = getFileCache;
    u.getConfig = getConfig;
    u.addConfig = addConfig;
    return u;
}


/* utility methods */

void update_cache(){
    cache = json_object_from_file("cache.json");
    if (cache == NULL){
        cache = json_object_new_object();
    }
}

void update_config(){
    config = json_object_from_file("config.json");
    if (config == NULL){
        config = json_object_new_object();
    }
}

void push_cache(){
    printf("pushing cache\n");
    json_object_to_file("cache.json",cache);
}

void push_config(){
    json_object_to_file("config.json",config);
}
