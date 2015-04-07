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
