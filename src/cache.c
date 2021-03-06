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
#include <err.h>
#include <unistd.h>
#include <limits.h>
#include "ipc_semaphore.h"
#include "log.h"

#ifndef MAP_FILE
#define MAP_FILE 0
#endif

json_object * cache;
json_object * config;

char * cacheFile;
int cacheFD;
long long int * cacheVersion;
size_t cacheLength;
semaphore cache_semaphore;
semaphore config_semaphore;

/* definition of local utility functions */
void update_cache();
void update_config();
void push_config();
void push_cache();

/* public functions */
void cache_init(){
    //set up shared memory
    cacheLength = 10 * 1024 * 1024; // 100 MB for now.
    char path[PATH_MAX];
    strcpy(path, getenv("HOME"));
    strcat(path, "/.cache/syncd/cache.json");
    int cacheFD = open(path,O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
    ftruncate(cacheFD,cacheLength);
    logging_log(LOGARGS,"fd = %d\n",cacheFD);
    cacheFile = (char *) mmap(NULL, cacheLength,
            PROT_READ | PROT_WRITE, MAP_FILE|MAP_SHARED, cacheFD, 0);
    if (cacheFile == MAP_FAILED ) errx(1,"failed");
    // setup semaphores.
    cache_semaphore = semaphore_create(1);
    config_semaphore = semaphore_create(1);
}

void cache_clear(){
    munmap(cacheFile, cacheLength);
    close(cacheFD);
    semaphore_delete(cache_semaphore);
    semaphore_delete(config_semaphore);
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
        logging_log(LOGARGS," failure\n");
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
    json_object * cache_entry = json_object_get(entry);
    json_object * pcache = getCache(plugin_prefix);
    if (pcache == NULL){/* create cache for plugin*/
        pcache = json_object_new_object();
        json_object_object_add(cache, plugin_prefix, pcache);
    }
    if (fname != NULL && strlen(fname) != 0 && cache_entry != NULL){
        json_object_object_add(pcache, fname, cache_entry);
        push_cache();
    } else {
        logging_log(LOGARGS,"addaCache Failed: fname = %s; cache_entry = %s\n",fname,json_object_to_json_string(cache_entry));
    }
    json_object_put(cache_entry);
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
    semaphore_wait(cache_semaphore);
    cache = json_tokener_parse(cacheFile);
    semaphore_post(cache_semaphore);

    // cache = json_object_from_file("cache.json");
    if (cache == NULL){
        cache = json_object_new_object();
    }
}

void update_config(){
    if (config != NULL) json_object_put(config);
    char path[PATH_MAX];
    strcpy(path,getenv("HOME"));
    strcat(path,"/.config/syncd/config.json");

    semaphore_wait(config_semaphore);
    config = json_object_from_file(path);
    semaphore_post(config_semaphore);

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
    semaphore_wait(cache_semaphore);
    memcpy(cacheFile, string, strlen(string));
    msync(cacheFile,strlen(string),MS_SYNC|MS_INVALIDATE);
    semaphore_post(cache_semaphore);
    logging_log(LOGARGS,"cache pushed\n");
}

void push_config(){
    char path[PATH_MAX];
    strcpy(path,getenv("HOME"));
    strcat(path,"/.config/syncd/config.json");
    semaphore_wait(config_semaphore);
    json_object_to_file(path,config);
    semaphore_post(config_semaphore);
}
