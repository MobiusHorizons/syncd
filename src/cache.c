/*
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

#define _XOPEN_SOURCE 500
#include "cache.h"
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include "lock.h"
#include "shared_memory.h"
#include "log.h"
#include <config.h>
#include <json-c/json_object_private.h>

#ifndef MAP_FILE
#define MAP_FILE 0
#endif

#ifndef HAVE_ERR_H
void errx(int i,const char * in){
    return;
}
#else
#include <err.h>
#endif

json_object * cache;
json_object * config;

char * cacheFile;
int cacheFD;
int configFD;

long long int * cacheVersion;
size_t cacheLength;
syncd_lock cache_lock;
syncd_lock config_lock;

/* definition of local utility functions */
void update_cache();
void update_config();
void push_config();
void push_cache();

/* public functions */
void cache_init(){
    //set up shared memory
    //cacheLength = (size_t *) shared_mem_alloc(sizeof(size_t), "SYNCD\\cache_length");
    cacheLength = 10 * 1024 * 1024; // 10 MB for now.
    char path[PATH_MAX];

	  strcpy(path, getenv("HOME"));
    strcat(path, "/.cache/syncd/cache.json");
    int cacheFD = open(path,O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
    cache_lock  = syncd_lock_create(path, cacheFD);
    ftruncate(cacheFD,cacheLength);
    logging_log(LOGARGS,"fd = %d\n",cacheFD);
    cacheFile = (char *) mmap(NULL, cacheLength,
            PROT_READ | PROT_WRITE, MAP_FILE|MAP_SHARED, cacheFD, 0);
    if (cacheFile == MAP_FAILED ) errx(1,"failed");


	  strcpy(path, getenv("HOME"));
    strcat(path, "/.cache/syncd/config.json");
    int configFD = open(path,O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
    config_lock  = syncd_lock_create(path, configFD);
}

void cache_clear(){
    munmap(cacheFile, cacheLength);
    close(cacheFD);
		json_object_put(config);
	  json_object_put(cache);
    config = NULL;
    cache = NULL;
    syncd_lock_delete(cache_lock );
    syncd_lock_delete(config_lock);
}

json_object * getCache(const char * plugin_prefix){
    json_object * pluginCache;
    update_cache(); // force synchronization.
    //logging_log(LOGARGS,"cache : %s\n",json_object_to_json_string(cache));
    //logging_log(LOGARGS,"got cache\n");
    if (json_object_object_get_ex(cache, plugin_prefix, &pluginCache)){
        return pluginCache;
    }
    return NULL;
}


json_object * getFileCache(const char * plugin_prefix,const char * fname){
    json_object * pcache = getCache(plugin_prefix);
    logging_log(LOGARGS,"getFileCache(\"%s\",\"%s\") \n",plugin_prefix,fname);
    json_object * fcache;
    if (pcache != NULL && json_object_object_get_ex(pcache, fname, &fcache)){
        logging_log(LOGARGS,"%s\n",json_object_to_json_string(fcache));
        return fcache;
    } else {
        logging_log(LOGARGS," cache miss.\n");
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

void addCache(const char * plugin_prefix, const char * fname, json_object * entry){
	  printf("addCache: Refcount = %d\n", entry->_ref_count);
    json_object * cache_entry = json_object_get(entry);
    json_object * pcache = getCache(plugin_prefix);
    if (pcache == NULL){/* create cache for plugin*/
        pcache = json_object_new_object();
        json_object_object_add(cache, plugin_prefix, pcache);
    }
    if (fname != NULL && strlen(fname) != 0 && cache_entry != NULL){
				if (json_object_object_get_ex(pcache, fname, NULL)){
					json_object_object_del(pcache, fname);
				}
        json_object_object_add(pcache, fname, cache_entry);
        push_cache();
    } else {
        printf("addCache Failed: fname = %s; cache_entry = %s\n",fname,json_object_to_json_string(cache_entry));
    }
    json_object_put(cache_entry);
	  printf("addCache: Refcount = %d\n", entry->_ref_count);
}

void updateCache(const char * plugin_prefix, json_object * pcache){
    json_object_get(pcache);
    update_cache();
    json_object_object_add(cache, plugin_prefix, pcache);
    push_cache();
    json_object_put(pcache);
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
    json_object_get(pconfig);
    update_config();
    json_object_object_add(config, plugin_prefix, pconfig);
    push_config();
    json_object_put(pconfig);
}

utilities get_utility_functions(){
    utilities u;
    u.getCache = getCache;
    u.addCache = addCache;
    u.updateFileCache = updateFileCache;
    u.updateCache = updateCache;
    u.getFileCache = getFileCache;
    u.getConfig = getConfig;
    u.addConfig = addConfig;
    return u;
}


/* utility methods */

void update_cache(){
    if (cache != NULL) json_object_put(cache);
    syncd_lock_wait(cache_lock, lock_shared);
    cache = json_tokener_parse(cacheFile);
    syncd_lock_release(cache_lock, lock_shared);

    // cache = json_object_from_file("cache.json");
    if (cache == NULL){
        cache = json_object_new_object();
    }
}

void update_config(){
    if (config != NULL) {
        logging_log(LOGARGS,"%s\n",json_object_to_json_string_ext(config, JSON_C_TO_STRING_PRETTY));
        json_object_put(config);
        config = NULL;
    }
    char path[PATH_MAX];
    strcpy(path,getenv("HOME"));
    strcat(path,"/.config/syncd/config.json");

    syncd_lock_wait(config_lock, lock_shared);
    config = json_object_from_file(path);
    syncd_lock_release(config_lock, lock_shared);

    if (config == NULL){
        config = json_object_new_object();
    }
}

void push_cache(){
    //logging_log(LOGARGS,"pushing cache\n");
    //json_object_to_file("cache.json",cache);
    const char * string = "";
    if (cache != NULL ) string = json_object_to_json_string(cache);
    logging_log(LOGARGS,"cache.json length = %d\n",(int)strlen(string));
    if (strlen(string) > cacheLength){
        logging_log(LOGARGS,"the string is too long\n");
    }
    syncd_lock_wait(cache_lock, lock_exclusive);
    memcpy(cacheFile, string, strlen(string));
    msync(cacheFile,strlen(string),MS_SYNC|MS_INVALIDATE);
    syncd_lock_release(cache_lock, lock_exclusive);
    logging_log(LOGARGS,"cache pushed\n");
}

void push_config(){
    char path[PATH_MAX];
    strcpy(path,getenv("HOME"));
    strcat(path,"/.config/syncd/config.json");
    syncd_lock_wait(config_lock, lock_exclusive);
    json_object_to_file(path,config);
    syncd_lock_release(config_lock, lock_exclusive);
}
