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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "dropbox_api.h"
#include "dropbox_urls.h"


FILE * db_files_get(const char* path, const char* access_token){
	char* params[2];
	char * url = malloc(strlen(path) + strlen(FILES_GET) + 2);

	rest_build_param(&params[0],"access_token",access_token);
	sprintf(url, "%s%s", FILES_GET, path);
	params[1] = NULL;

	FILE * file = rest_get(params,url);
	free(url);
	free(params[0]);
	return file;
}

json_object * db_files_put(const char* path, const char* access_token, FILE * input_file){
	char* params[2];
	char * url = malloc(strlen(path) + strlen(FILES_PUT) + 2);

	rest_build_param(&params[0],"access_token",access_token);
	sprintf(url, "%s%s", FILES_PUT, path);
	params[1] = NULL;

	buffer resp = rest_put_file(params,url,input_file);
	json_object * response = json_tokener_parse(resp.data);

	buffer_free(resp);
	free(url);
	free(params[0]);

	return response;
}

json_object * db_metadata (const char* path, const char* access_token, bool list){
	char * params[3];
	int i;
	rest_build_param(&params[0],"access_token",access_token);
	char * url = malloc(strlen(path) + strlen(METADATA)+1);
	strcpy(url, METADATA);
	strcat(url, path);
	params[1] = list?"list=true":"list=false";
	params[2]=NULL;

	buffer resp= rest_get_buffer(params,url);
	json_object * response = json_tokener_parse(resp.data);
//	printf(json_object_to_json_string(response));

	buffer_free(resp);
	free(params[0]);
	free(url);
	return response;
}

json_object * db_longpoll (const char* cursor, int timeout){
	char * params[3];
	int i;
	if (timeout > 480) timeout = 480;// max allowed
	if (timeout < 30)  timeout = 30;  // minimum allowed
	rest_build_param(&params[0],"cursor",cursor);
	params[1] = malloc(3 + strlen("timeout=")+1);
	sprintf(params[1],"timeout=%d",timeout);
	params[2] = NULL;

	buffer resp = rest_get_buffer(params,LONGPOLL);
	json_object * response = json_tokener_parse(resp.data);
//	printf(json_object_to_json_string(response));

	buffer_free(resp);
	free (params[0]);
	free (params[1]);
	return response;
}

json_object * db_delta    (char* cursor, const char* access_token){
	char* params[3];
	if (cursor != NULL){
		rest_build_param(&params[1],"cursor",cursor);
	} else {
		params[1] = NULL;
	}
	rest_build_param(&params[0],"access_token",access_token);
	params[2] = NULL;

	buffer resp = rest_post(params,DELTA);
	json_object * response = json_tokener_parse(resp.data);

	buffer_free(resp);
	free(params[0]);
	free(params[1]);
	return response;
}

char * db_authorize_token (char* token, char * client_id, char* client_secret){
	char * params[5];
	json_object *access_token;

	rest_build_param(&params[0], "code",token);
	rest_build_param(&params[1], "client_id",client_id);
	rest_build_param(&params[2], "client_secret",client_secret);
	rest_build_param(&params[3], "grant_type","authorization_code");
	params[4] = NULL;

	buffer resp = rest_post(params,OAUTH_2_TOKEN);
	json_object * response = json_tokener_parse(resp.data);

	buffer_free(resp);
	free(params[0]);
	free(params[1]);
	free(params[2]);
	free(params[3]);

	if (json_object_object_get_ex(response,"access_token",&access_token)){

	  char * at = (char*)(json_object_get_string(access_token));
	  if (at != NULL) at = strdup(at);
	  json_object_put(response);
		return at;
	}
  json_object_put(response);
	//printf("%s\n",json_object_to_json_string(response));
	return NULL;
}

json_object * db_mkdir(const char* name, const char * access_token){
	char * params[4];
	params[0] = "root=dropbox";
	rest_build_param(&params[1], "path",name);
	rest_build_param(&params[2], "access_token", access_token);
	params[3] = NULL;

	buffer resp = rest_post(params, FILES_MKDIR);
	json_object * response = json_tokener_parse(resp.data);

	buffer_free(resp);
	free(params[1]);
	free(params[2]);
	return response;
}

json_object * db_mv(const char* from, const char * to, const char * access_token){
	char * params[5];
	params[0] = "root=dropbox";
	rest_build_param(&params[1], "from_path",from);
	rest_build_param(&params[2], "to_path", to);
	rest_build_param(&params[3], "access_token", access_token);
	params[4] = NULL;

	buffer resp = rest_post(params, FILES_MV);
	json_object * response = json_tokener_parse(resp.data);

	buffer_free(resp);
	free(params[1]);
	free(params[2]);
	free(params[3]);
	return response;
}

json_object * db_rm(const char* name, const char * access_token){
	char * params[4];
	params[0] = "root=dropbox";
	rest_build_param(&params[1], "path",name);
	rest_build_param(&params[2], "access_token", access_token);
	params[3] = NULL;

	buffer resp = rest_post(params, FILES_RM );
	json_object * response = json_tokener_parse(resp.data);

	buffer_free(resp);
	free(params[1]);
	free(params[2]);
	return response;
}
