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

#ifndef __LIBREST_H__
#define __LIBREST_H__
#include <curl/curl.h>
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
