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

#define _XOPEN_SOURCE 500
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include "../libdropbox/dropbox_api.h"

#define PLUGIN_PREFIX "dropbox://"
#define PLUGIN_PREFIX_LEN 10 // i counted
#include <src/plugin.h>


#if defined(_WIN32)
#define URL_OPEN_CMD "start \"\""
#define SILENT_CMD ""
#elif defined(__APPLE__) && defined(__MACH__)
#define URL_OPEN_CMD "open"
#define SILENT_CMD ""
#else
#define URL_OPEN_CMD "sh -c 'xdg-open"
#define SILENT_CMD "' >/dev/null 2&>/dev/null"
#endif

/* globals */
char * client_key    = "gmq6fs74fuw1ead";
char * client_secret = "ia87pt0ep6dvb7y";
char * access_token;
init_args args;
utilities utils;
json_object * config;
json_object * cache;

/* convenience functions */
const char * JSON_GET_STRING(json_object * obj, char * object){
  if (json_object_object_get_ex(obj,object,&obj)){
    return json_object_get_string(obj);
  }
  return NULL;
}

bool JSON_GET_BOOL(json_object * obj, char * object, bool def){
  if (json_object_object_get_ex(obj,object,&obj)){
    return json_object_get_boolean(obj);
  }
  return def;
}

char * safe_strdup(const char * str){
  if (str != NULL){
    return strdup(str);
  } else {
    return NULL;
  }
}

void update_cache(json_object * entry,const char *fname){
  json_object * cfile = utils.getFileCache(PLUGIN_PREFIX,fname);
  if (cfile == NULL)
  cfile = json_object_new_object();

  const char * old_rev = json_get_string(cfile,"rev");
  if (old_rev == NULL) old_rev = "initial";
  long long int old_ver = json_get_int(cfile,"version",1);

  if (entry == NULL) { // the file was deleted
    time_t modified;
    time(&modified);
    entry = json_object_new_object();
    json_object_object_add(entry, "modified", json_object_new_int64((long long)modified));
    json_object_object_add(entry, "rev", json_object_new_string("deleted"));
    json_object_object_add(entry, "size", json_object_new_int64(0));
  }
  args.log(LOGARGS,"update_cache : %s\n",json_object_to_json_string_ext(entry, JSON_C_TO_STRING_PRETTY));
  struct tm mod = {0};
  time_t modified;
  const char * modified_s = JSON_GET_STRING(entry,"modified");
  const char * new_rev = json_get_string(entry,"rev");
  if (new_rev == NULL) new_rev = "blank";

  if (strcmp(old_rev,new_rev)<=0) old_ver++;
  strptime(modified_s,"%a, %d %b %Y %H:%M:%S %z",&mod);
  modified = mktime(&mod); // convert struct tm to time_t (ie seconds since epoch).
  json_object * size;
  if (!json_object_object_get_ex(entry, "bytes", &size)){
    args.log(LOGARGS,"entry has no size object\n");
  }
  size = json_object_new_int64(json_object_get_int64(size)); // copy
  args.log(LOGARGS,"size : %ld\n",json_object_get_int64(size));

  //    json_copy(&cfile, "rev", entry, json_object_new_string("blank"));
  {
    const char *rev;
    if ((rev = json_get_string(entry,"rev"))==NULL){
      rev = "blank";
    }
    json_object_object_add(cfile, "rev", json_object_new_string(rev));
  }
  json_object_object_add(cfile, "version", json_object_new_int64(old_ver));
  json_object_object_add(cfile, "modified", json_object_new_int64((long long)modified));
  json_object_object_add(cfile, "size", size);
  args.log(LOGARGS,"'%s' modified :%ld, size: %ld bytes\n", fname, modified, json_object_get_int64(size));

  utils.addCache(PLUGIN_PREFIX, fname, cfile);
}

const char * get_prefix(){
  return PLUGIN_PREFIX;
}

char * init(init_args a){
  args = a;
  utils = args.utils;
  curl_global_init(CURL_GLOBAL_DEFAULT);
  config = utils.getConfig(PLUGIN_PREFIX);
  if (config == NULL){
    config = json_object_new_object();
  }
  access_token = safe_strdup(json_get_string(config, "access_token"));
  FILE * state = fopen("access_token.txt", "r");
  if (access_token == NULL){
    char  token[128];
    char cmd[512];
    sprintf(cmd, "%s \"%s?response_type=code&client_id=%s\" %s",URL_OPEN_CMD,OAUTH_2_AUTH,client_key, SILENT_CMD);
    //printf("go to https://www.dropbox.com/1/oauth2/authorize?response_type=code&client_id=%s and copy the code here\n",client_key);
    args.stdout("Opening a browser to authorize this app for use with DropBox. Please paste the token here\n");
    int cmd_ret = system(cmd);
    args.log(LOGARGS, "command returned %d\n", WEXITSTATUS(cmd_ret));
    if(WEXITSTATUS(cmd_ret) != 0){
      // We don't have a browser.
      args.log(LOGARGS,"No browser, so we will put the url in the command line.\n");
      args.stdout("oops, you don't seem to have a browser.\n");
      args.stdout("Please go to %s?response_type=code&client_id=%s and log in.\n", OAUTH_2_AUTH, client_key);
    }
    if (fgets(token,128,stdin) == NULL) exit(1);
    int len = strlen(token);
    if (token[len-1] == '\n') token[len-1] = '\0';
    access_token = safe_strdup(db_authorize_token(token,client_key,client_secret));
    if (access_token == NULL) exit(1);
    json_object * at = json_object_new_string(access_token);
    args.log(LOGARGS,"config = %s\n",json_object_to_json_string(config));
    json_object_object_add(config, "access_token", at);
    utils.addConfig(PLUGIN_PREFIX, config);
  }

  return PLUGIN_PREFIX;
}

void sync_unload(){
  curl_global_cleanup();
}


void sync_listen(int (*cb)(const char*,int)){
  CATCH_EVENTS
  static char * cursor;
  {
    json_object * jcursor;
    config = utils.getConfig(PLUGIN_PREFIX);
    if (json_object_object_get_ex(config, "cursor", &jcursor)){
      cursor = strdup(json_object_get_string(jcursor));
    }
  }
  args.log(LOGARGS,"cursor=%s\n",cursor);
  bool has_more;
  int i;
  while (true){
    do {
      CLEAN_BREAK
      char path[PATH_MAX];
      json_object * delta = db_delta(cursor,access_token);
      char * new_cursor;
      {
        const char * c;
        if ((c = json_get_string(delta,"cursor"))!= NULL){
          //free (cursor);
          new_cursor = strdup(c);
        } else {
          args.log(LOGARGS,"delta = %s\n",json_object_to_json_string(delta));

          free(cursor);
          cursor = NULL;
          continue;
        }
      }
      json_object* entry;
      json_object *entries;
      if (!json_object_object_get_ex(delta,"entries",&entries)){
        args.log(LOGARGS,"no delta['entries']\n");
        free(cursor);
        cursor=NULL;
        continue;
      }

      for (i = 0; i < json_object_array_length(entries); i++){ // look through all entries
        entry = json_object_array_get_idx(entries,i);
        char lowercasePath[PATH_MAX];
        strcpy(lowercasePath,json_object_get_string(json_object_array_get_idx(entry,0)));
        args.log(LOGARGS,"%s\n",json_object_to_json_string(entry));

        if (! json_object_is_type(json_object_array_get_idx(entry,1), json_type_null)){
          entry = json_object_get(json_object_array_get_idx(entry,1));
          sprintf(path,"%s%s",PLUGIN_PREFIX,json_get_string(entry, "path"));
          json_object* path_map = json_object_new_object();
          json_object_object_add(path_map, "path", json_object_new_string(path));
          utils.addCache(PLUGIN_PREFIX,lowercasePath,path_map);
          args.log(LOGARGS,"adding cache entry for pathmap %s => %s \n", lowercasePath, path);
          update_cache(entry, path + PLUGIN_PREFIX_LEN);

          if (JSON_GET_BOOL(entry,"is_dir",false))
          cb(path, S_DIR | S_CREATE);
          else
          cb(path,S_CLOSE_WRITE);
        } else {
          json_object * path_map = utils.getFileCache(PLUGIN_PREFIX, lowercasePath);
          if (path_map != NULL){
            strcpy(path, json_get_string(path_map, "path"));
            // delete
            update_cache(NULL,path + PLUGIN_PREFIX_LEN);
          }
          cb(path,S_DELETE);
        }

        //json_object_put(entry); // free the entry
      }

      has_more = JSON_GET_BOOL(delta,"has_more",false);

      args.log(LOGARGS,"========================================================\n\n");
      args.log(LOGARGS,"Old Cursor : %s\n",cursor);
      free(cursor);
      cursor = new_cursor;
      args.log(LOGARGS,"New Cursor : %s\n\n",new_cursor);
      args.log(LOGARGS,"========================================================\n\n");

      config = utils.getConfig(PLUGIN_PREFIX);
      json_object_object_add(config, "cursor", json_object_new_string(cursor));
      utils.addConfig(PLUGIN_PREFIX,config);

      json_object_put(delta);
    } while (has_more);
    bool changes;
    do {// wait for updates
      if (cursor == NULL) break;
      json_object * resp;
      args.log(LOGARGS,"waiting for changes, 120 sec\n");
      resp = db_longpoll(cursor,120);
      args.log(LOGARGS,"%s\n",json_object_to_json_string(resp));
      changes = JSON_GET_BOOL(resp,"changes",false);
      json_object_put(resp);
      CLEAN_BREAK
    } while (!changes && cursor != NULL);
  }
}

void do_poll(int interval){
  args.log(LOGARGS,"sleeping %ld seconds\n",interval-(time(NULL)%interval));
  sleep (interval - (time(NULL)%interval));
  //	update();
}


void watch_dir(char*path){
  args.log(LOGARGS,"i am supposed to be monitoring directory %s\n",path);
}

FILE * sync_open(const char*ipath){
  char * path = strdup(ipath + PLUGIN_PREFIX_LEN);
  FILE * r = db_files_get(path,access_token);
  free(path);
  return r;
}

void sync_close(FILE *fp){ // deprecated
  fclose(fp);
}

int sync_read (int id, char * buffer, int len){ // deprecated
  return 0;
}

int sync_write(char * path, FILE * fp){
  args.log(LOGARGS,"path = '%s', file = %p\n",path,fp);
  path += PLUGIN_PREFIX_LEN;
  json_object * metadata = db_files_put(path,access_token,fp);
  update_cache(metadata,path);
  json_object_put(metadata);
  return 1;
}

int sync_mkdir(char* path){
  path += PLUGIN_PREFIX_LEN;
  db_mkdir(path,access_token);
  return 0;
}

int sync_rm(char * path){
  path += PLUGIN_PREFIX_LEN;
  json_object * response = db_rm(path,access_token);

	args.log(LOGARGS, json_object_to_json_string(response));
	json_object_put(response);

  return 0;
}

int sync_mv(char * from, char* to){
  from += PLUGIN_PREFIX_LEN;
  to   += PLUGIN_PREFIX_LEN;
  db_mv(from,to,access_token);
  return 0;
}
