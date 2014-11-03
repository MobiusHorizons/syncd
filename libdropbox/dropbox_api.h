#ifndef __DB_API_H__
#define __DB_API_H__
#ifdef WIN32
#	include "curl/include/curl/curl.h"	
#else
#	include <curl/curl.h>
#endif
#include <json-c/json.h>
#include <stdbool.h>
#include "dropbox_urls.h"
#include "librest/rest.h"

FILE * db_files_get(char* path, const char* access_token);
json_object * db_files_put(char* path, const char* access_token, FILE *   input_file);
json_object * db_metadata (char* path, const char* access_token, bool list);
json_object * db_delta    (char* cursor, const char* access_token);
json_object * db_longpoll (char* cursor,int timeout);
const char * db_authorize_token (char* token, char * client_id, char* client_secret);
json_object * db_mkdir(char * name, const char * access_token);
json_object * db_mv(char * from,char * to, const char * access_token);
json_object * db_rm(char * name, const char * access_token);
#endif

