#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <dropbox_api.h>
#include "../cache.h"

#define PLUGIN_PREFIX "dropbox://"
#define PLUGIN_PREFIX_LEN 10 // i counted

/* globals */
char * client_key    = "gmq6fs74fuw1ead";
char * client_secret = "ia87pt0ep6dvb7y";
const char * access_token;
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


void update_cache(json_object * entry,const char *fname){
    if (entry == NULL) { // the file was deleted
        time_t modified;
        time(&modified);
        entry = json_object_new_object();
        json_object_object_add(entry, "modified", json_object_new_int64((long long)modified));
        json_object_object_add(entry, "size", json_object_new_int64(0));
    }
    //todo : add better error handling (what to do if size/modified doesn't exist)
    json_object * cfile = utils.getFileCache(PLUGIN_PREFIX,fname);
    if (cfile == NULL)
        cfile = json_object_new_object();
    printf("update_cache : %s\n",json_object_to_json_string_ext(entry, JSON_C_TO_STRING_PRETTY));
    struct tm mod = {0};
    time_t modified;
    const char * modified_s = JSON_GET_STRING(entry,"modified");
    printf ("modified : %s\n",modified_s);
    strptime(modified_s,"%a, %d %b %Y %H:%M:%S %z",&mod);
    modified = mktime(&mod);
    printf("modified : %d\n", modified);
    json_object * size;
    if (!json_object_object_get_ex(entry, "bytes", &size)){
        printf("entry has no size object\n");
    } 
    size = json_object_new_int64(json_object_get_int64(size));
    printf("size : %d\n",json_object_get_int64(size));

    /*if (json_object_object_get_ex(cfile, "modified", NULL)){
        json_object_object_del(cfile, "modified");
        printf ("deleted the modified object\n");
    } else {
        printf ("did not have to delete modified\n");
    }*/
    json_object_object_add(cfile, "modified", json_object_new_int64((long long)modified));
    json_object_object_add(cfile, "size", size);
    printf("'%s' modified :%d, size: %d bytes\n", fname, modified, json_object_get_int64(size));

    utils.addCache(PLUGIN_PREFIX, fname, cfile);
}

char * init(utilities u){
    utils = u;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    config = u.getConfig(PLUGIN_PREFIX);
    if (config == NULL){
        config = json_object_new_object();
    }
    access_token = JSON_GET_STRING(config, "access_token");
    FILE * state = fopen("access_token.txt", "r");
    if (access_token == NULL){
        char  token[128];
        printf("go to https://www.dropbox.com/1/oauth2/authorize?response_type=code&client_id=%s and copy the code here\n",client_key);
        if (fgets(token,128,stdin) == NULL) exit(1);
        int len = strlen(token);
        if (token[len-1] == '\n') token[len-1] = '\0';
        access_token = db_authorize_token(token,client_key,client_secret);
        if (access_token == NULL) exit(1);
        json_object * at = json_object_new_string(access_token);
        printf("config = %s\n",json_object_to_json_string(config));
        json_object_object_add(config, "access_token", at);
        u.addConfig(PLUGIN_PREFIX, config);
    }

    return PLUGIN_PREFIX;
}

void sync_unload(){
	curl_global_cleanup();
}


void sync_listen(int (*cb)(const char*,int)){
	static char * cursor;
	bool has_more;
	int i;
	while (true){
	do {
		char path[PATH_MAX];
		json_object * delta = db_delta(cursor,access_token);
		free(cursor);
		cursor = strdup(JSON_GET_STRING(delta,"cursor"));
		json_object* entry; 
		json_object *entries;
		if (!json_object_object_get_ex(delta,"entries",&entries)){
			printf("no delta['entries']\n");
			return;
		}
	
		for (i = 0; i < json_object_array_length(entries); i++){ // look through all entries
			entry = json_object_array_get_idx(entries,i);
			printf("%s\n",json_object_to_json_string(entry));
			sprintf(path,"%s%s",PLUGIN_PREFIX,json_object_get_string(json_object_array_get_idx(entry,0)));
			printf("path = %s\n",path);
			if (json_object_array_length(entry)==2){
				entry = json_object_array_get_idx(entry,1);
                update_cache(entry, path + PLUGIN_PREFIX_LEN);
				if (JSON_GET_BOOL(entry,"is_dir",false))
					cb(path,0x40000100);
				else 
					cb(path,0x08);
			} else {
                // delete
                update_cache(NULL,path + PLUGIN_PREFIX_LEN);
				cb(path,0x200);
			}

			json_object_put(entry); // free the entry
		}

		has_more = JSON_GET_BOOL(delta,"has_more",false);

		json_object_put(entries); // free
		json_object_put(delta); 	
	} while (has_more);

	bool changes;
	do {// wait for updates
		printf("waiting for changes, 120 sec\n");
		json_object * resp = db_longpoll(cursor,120);
		printf("%s\n",json_object_to_json_string(resp));
		changes = JSON_GET_BOOL(resp,"changes",false);
		json_object_put(resp);
	} while (!changes);
	}
}

void do_poll(int interval){
	printf("sleeping %d seconds\n",interval-(time(NULL)%interval));
	sleep (interval - (time(NULL)%interval));
//	update();
}


void watch_dir(char*path){
	printf("i am supposed to be monitoring directory %s\n",path);
}

FILE * sync_open(const char*ipath){
	char * path = ipath + PLUGIN_PREFIX_LEN;
	return db_files_get(path,access_token);
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
	json_object_put(metadata);
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

