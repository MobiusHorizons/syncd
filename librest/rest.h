#ifndef __LIBREST_H__
#define __LIBREST_H__
#ifdef WIN32
#	include "curl/include/curl/curl.h"	
#else
#	include <curl/curl.h>
#endif
#include <json-c/json.h>
#include <stdbool.h>
#include "buffer.h"



typedef struct {
	CURL * curl;
	FILE * file;
	char * url;
} run_curl_args;

typedef struct {
	char ** params;
	char ** headers;
	char * url;
	char * content_type;
	buffer * content;
	char ** return_headers;
	buffer * return_data;
} rest_args;

int    		EXPORT_SHARED rest_build_param(char** param, const char * name, const char* value);
int    		EXPORT_SHARED rest_build_header(char** header, const char * name, const char* value);
char * 		EXPORT_SHARED rest_build_url(char ** params, char* base);
char * 		EXPORT_SHARED rest_escape(char * url);
void * 		EXPORT_SHARED run_curl(void* ptr);
//size_t 		EXPORT_SHARED ReadFileCB( void *contents, size_t size, size_t nmemb, void *userp);
//static size_t 	EXPORT_SHARED WriteFileCB(void * contents, size_t size, size_t nmemb, void * userp);
//static size_t 	EXPORT_SHARED WriteBufferCB(void *contents, size_t size, size_t nmemb, void *userp);
FILE * 		EXPORT_SHARED rest_get	(char ** params, char * url);
buffer 		EXPORT_SHARED rest_get_buffer (char ** params, char * url);
buffer 		EXPORT_SHARED rest_post (char ** params, char * url);
buffer 		EXPORT_SHARED rest_post_headers (char ** headers,char ** params, char * url);
int 		EXPORT_SHARED rest_post_all(rest_args args);
buffer 		EXPORT_SHARED rest_put_file (char** params, char* url, FILE * in);
buffer 		EXPORT_SHARED rest_put_headers (char** params, char** headers,char* url, FILE * in);
#endif
