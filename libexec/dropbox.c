#define _XOPEN_SOURCE 500
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../libdropbox/dropbox_api.h"
#include <src/plugin.h>

#define PLUGIN_PREFIX "dropbox://"
#define PLUGIN_PREFIX_LEN 10 // i counted

/* globals */
char * client_key    = "gmq6fs74fuw1ead";
char * client_secret = "ia87pt0ep6dvb7y";
char * access_token;
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
    printf("update_cache : %s\n",json_object_to_json_string_ext(entry, JSON_C_TO_STRING_PRETTY));
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
        printf("entry has no size object\n");
    }
    size = json_object_new_int64(json_object_get_int64(size)); // copy
    printf("size : %ld\n",json_object_get_int64(size));

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
    printf("'%s' modified :%ld, size: %ld bytes\n", fname, modified, json_object_get_int64(size));

    utils.addCache(PLUGIN_PREFIX, fname, cfile);
}

char * init(init_args args){
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
        printf("go to https://www.dropbox.com/1/oauth2/authorize?response_type=code&client_id=%s and copy the code here\n",client_key);
        if (fgets(token,128,stdin) == NULL) exit(1);
        int len = strlen(token);
        if (token[len-1] == '\n') token[len-1] = '\0';
        access_token = safe_strdup(db_authorize_token(token,client_key,client_secret));
        if (access_token == NULL) exit(1);
        json_object * at = json_object_new_string(access_token);
        printf("config = %s\n",json_object_to_json_string(config));
        json_object_object_add(config, "access_token", at);
        utils.addConfig(PLUGIN_PREFIX, config);
    }

    return PLUGIN_PREFIX;
}

void sync_unload(){
	curl_global_cleanup();
}


void sync_listen(int (*cb)(const char*,int)){
	static char * cursor;
    {
        json_object * jcursor;
        config = utils.getConfig(PLUGIN_PREFIX);
        if (json_object_object_get_ex(config, "cursor", &jcursor)){
            cursor = strdup(json_object_get_string(jcursor));
        }
    }
    printf("cursor=%s\n",cursor);
	bool has_more;
	int i;
	while (true){
	do {
		char path[PATH_MAX];
		json_object * delta = db_delta(cursor,access_token);
		char * new_cursor;
        {
            const char * c;
            if ((c = json_get_string(delta,"cursor"))!= NULL){
                //free (cursor);
                new_cursor = strdup(c);
            } else {
                printf("delta = %s\n",json_object_to_json_string(delta));

                free(cursor);
                cursor = NULL;
                continue;
            }
        }
		json_object* entry;
		json_object *entries;
		if (!json_object_object_get_ex(delta,"entries",&entries)){
			printf("no delta['entries']\n");
            free(cursor);
            cursor=NULL;
			continue;
		}

		for (i = 0; i < json_object_array_length(entries); i++){ // look through all entries
			entry = json_object_array_get_idx(entries,i);
			char lowercasePath[PATH_MAX];
			strcpy(lowercasePath,json_object_get_string(json_object_array_get_idx(entry,0)));
			printf("%s\n",json_object_to_json_string(entry));
			//printf("path = %s\n",path);

			if (! json_object_is_type(json_object_array_get_idx(entry,1), json_type_null)){
					entry = json_object_get(json_object_array_get_idx(entry,1));
					sprintf(path,"%s%s",PLUGIN_PREFIX,json_get_string(entry, "path"));
					json_object* path_map = json_object_new_object();
					json_object_object_add(path_map, "path", json_object_new_string(path));
					utils.addCache(PLUGIN_PREFIX,lowercasePath,path_map);
					printf("adding cache entry for pathmap %s => %s \n", lowercasePath, path);
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

		printf("========================================================\n\n");
		printf("Old Cursor : %s\n",cursor);
		free(cursor);
		cursor = new_cursor;
		printf("New Cursor : %s\n\n",new_cursor);
		printf("========================================================\n\n");

		config = utils.getConfig(PLUGIN_PREFIX);
		json_object_object_add(config, "cursor", json_object_new_string(cursor));
		utils.addConfig(PLUGIN_PREFIX,config);

			json_object_put(delta);

		} while (has_more);

	bool changes;
	do {// wait for updates
        if (cursor == NULL) break;
        json_object * resp;
        printf("waiting for changes, 120 sec\n");
        resp = db_longpoll(cursor,120);
		printf("%s\n",json_object_to_json_string(resp));
		changes = JSON_GET_BOOL(resp,"changes",false);
		json_object_put(resp);
	} while (!changes && cursor != NULL);
	}
}

void do_poll(int interval){
	printf("sleeping %ld seconds\n",interval-(time(NULL)%interval));
	sleep (interval - (time(NULL)%interval));
//	update();
}


void watch_dir(char*path){
	printf("i am supposed to be monitoring directory %s\n",path);
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
	printf("path = '%s', file = %p\n",path,fp);
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
	db_rm(path,access_token);
	return 0;
}

int sync_mv(char * from, char* to){
	from += PLUGIN_PREFIX_LEN;
	to   += PLUGIN_PREFIX_LEN;
	db_mv(from,to,access_token);
	return 0;
}
