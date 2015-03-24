#define  _XOPEN_SOURCE 500
#include "gdrive_api.h"
#include <ctype.h>
#define __GLOBAL_CLIENT_ID 	"969830472849-93kt0dqjevn8jgr3g6erissiocdhk2fo.apps.googleusercontent.com"
#define __GLOBAL_CLIENT_SECRET	"gcyc89d--P9nUb1KagVeV496"
#define REFRESH_TOKEN "1/8obRmFxvhhebWSCYckmw_AfUlfTD-ERnwvoro8tMAKI"



/* globals*/
char * KEY;
json_object * cache;

/* convenience functions */
const char * JSON_GET_STRING(json_object * obj, const char * name){
	json_object * temp;
	if (json_object_object_get_ex(obj,name,&temp)){
		const char * ret = json_object_get_string(temp);
		return ret;
	}
	return NULL;
}

long JSON_GET_INT64(json_object * obj, const char * object){
	if (json_object_object_get_ex(obj,object,&obj)){
		return json_object_get_int64(obj);
	}
	return 0;
}

bool JSON_GET_BOOL(json_object * obj, const char * object, bool def){
	if (json_object_object_get_ex(obj,object,&obj)){
		return json_object_get_boolean(obj);
	}
	return def;
}

char * safe_strdup(const char * in){
  if (in == NULL) return NULL;
  return strdup(in);
}

json_object * gdrive_get_metadata(const char * file_id){
    printf("GDRIVE_GET_META: had to get medatada for id '%s'\n", file_id);
    char * params[2];
	char * url = (char*) malloc(strlen("https://www.googleapis.com/drive/v2/files/") + strlen(file_id) + 1);
	sprintf(url, "https://www.googleapis.com/drive/v2/files/%s",file_id);
//	printf("url = %s\n",url);
	rest_build_param(&params[0],"access_token",KEY);
	params[1] = NULL;
    buffer resp;
    do {
	    resp = rest_get_buffer(params,url);
    } while (resp.data == NULL);
	free(params[0]);
	free(url);
	json_object * g_metadata = json_tokener_parse(resp.data);
    free(resp.data);
    return g_metadata;
}

/*
json_object * gdrive_get_metadata(const char * file_id){
	json_object * ids_cache;
	if (!json_object_object_get_ex(cache,"ids", &ids_cache)){
		ids_cache = json_object_new_object();
		json_object_object_add(cache,"ids",ids_cache);
	}
	json_object * metadata;
	if (json_object_object_get_ex(ids_cache, file_id,&metadata)){
			return metadata;
	}
	metadata = _gdrive_get_metadata(file_id);
	json_object_object_add(ids_cache, file_id,metadata);
	return metadata;
}
*/

FILE * gdrive_get (const char * file_id){
    json_object * metadata = gdrive_get_metadata(file_id);
	char * download_url = safe_strdup(JSON_GET_STRING(metadata,"downloadUrl"));
    if (download_url == NULL) return NULL;
    download_url = (char*)realloc(download_url,
        strlen(download_url)
        + strlen("&access_token=")
        + strlen(KEY)
        + 1
    );
    strcat(download_url, "&access_token=");
    strcat(download_url, KEY);

    printf("download url = %s\n",download_url);
    FILE * ret = rest_get(NULL,download_url);
    free(download_url);
    json_object_put(metadata);
	return ret;
}

json_object * gdrive_get_changes(const char* pageToken,const char*startChangeId, int max, bool includeSubscribed, bool includeDeleted){
	char * params[7];
	int i = 0;
	rest_build_param(&params[i++], "access_token",KEY);
	if (max > 0){
		char max_str[4]; // max 1000
		sprintf(max_str,"%d",max);
		rest_build_param(&params[i++], "maxResults",max_str);
	}
	if (pageToken != NULL){
		rest_build_param(&params[i++], "pageToken",pageToken);
	}
	if (startChangeId != NULL){
		rest_build_param(&params[i++], "startChangeId",startChangeId);
	}
    rest_build_param(&params[i++], "includeDeleted", includeDeleted?"true":"false");
    rest_build_param(&params[i++], "includeSubscribed", includeSubscribed?"true":"false");
	params[i] = NULL;
	i = 0;
	while ( params[i] != NULL){
		printf("%s&",params[i++]);
	}
	printf("\n");
	buffer resp = rest_get_buffer(params,"https://www.googleapis.com/drive/v2/changes");
     
	i = 0;
	while (params[i] != NULL) free(params[i++]);
	
    if (resp.data == NULL) return NULL;
    json_object * response = json_tokener_parse(resp.data);
    buffer_free(resp);
    printf("returning response\n");
    return response;
}

json_object * gdrive_upload(char * url,FILE * file){
	char * bearer = (char*) malloc(strlen(KEY) + strlen("Bearer ")+1);
	char * headers[2];
    json_object * response = NULL;
	strcpy(bearer,"Bearer "); strcat(bearer,KEY);
	rest_build_header(&headers[0],"Authorization", bearer);
	free(bearer);
	headers[1] = NULL;
    
	buffer resp = rest_put_headers(NULL,headers,url,file);
//	printf("resp = %s\n",resp.data);
    if (resp.size > 0 && resp.data != NULL){
        response = json_tokener_parse(resp.data);
    }
	buffer_free(resp);
	free(headers[0]);
    return response;
}

char * get_folder_id(const char * dir, const char * parent_id){
	const char * format = "title = '%s' and "
		"mimeType = 'application/vnd.google-apps.folder' and "
		"'%s' in parents";
	printf (format,	dir,parent_id); printf("\n");
	char * q = (char*) malloc(strlen(format)+strlen(dir)+strlen(parent_id));
	sprintf(q,format,dir,parent_id);
	json_object * response = gdrive_files_list(q,0);
	json_object * items;
	json_object_object_get_ex(response,"items",&items);
	const char * temp_id = JSON_GET_STRING(json_object_array_get_idx(items,0),"id");
	printf("id = '%s'\n",temp_id);
	char * id = NULL;
	if (temp_id != NULL) id = strdup(temp_id);
	json_object_put(response);
	return id;
}

json_object * gdrive_files_put(const char * path, FILE * file){
	char * headers[2];
	char * bearer = malloc(strlen(KEY) + strlen("Bearer ")+1);
	char * return_headers[10];
	char * full_path = strdup(path);
	char * p = full_path;
	char * end = full_path + strlen(path); // points at the null terminator.
	char * fname;
	int i = 0;
	for (i = 0; i < 10; i ++){
		if (i == 9)return_headers[i] = "";
		else return_headers[i] = NULL;
	}
	json_object * parent_id;
	if (p[0] == '/'){ // valid path starts with "/"
		p++;
		char * dir = strdup("root");
		do {
			printf("'%s'\n",dir);
			// find next slash
			char * delim = p + strcspn(p,"/");
			while(delim[-1] == '\\'){
				int i=1;
				while (
					(delim - i) > p
					&& delim[-1*i] =='\\'
				) i++ ; // count '\'
				if (i%2 == 1) break;
				delim = delim +1 + strcspn(delim+1,"/");
			}
			if (delim == end) break;
			delim[0] = '\0';
			char * temp_dir = get_folder_id(p,dir);
			free(dir);
			dir = temp_dir;
			p = delim+1;
		} while (p < end);
		printf("id = '%s'\n",dir);
		parent_id = json_object_new_string(dir==NULL?"root":dir);
		fname = p;
	}
	strcpy(bearer,"Bearer "); strcat(bearer,KEY);
	rest_build_header(&headers[0],"Authorization", bearer);
	free(bearer);
//	rest_build_header(&headers[1],"X-Upload-Content-Type","application/");
//	printf("header = %s\n",headers[0]);
	headers[1] = NULL;

	/* set up arguments */
	rest_args args;
	args.url = "https://www.googleapis.com/upload/drive/v2/files?uploadType=resumable";
	args.headers = headers;
	args.params = NULL;
	args.content_type = NULL;
	args.content_type = "application/json; charset=UTF-8";
	json_object * metadata = json_object_new_object();
	json_object_object_add(
		metadata,
		"title",
		json_object_new_string(fname)
	);
	json_object * parent = json_object_new_object();
	json_object_object_add(
		parent,
		"id",
		parent_id
	);
	json_object * parents = json_object_new_array();
	json_object_array_add(parents,parent);
	json_object_object_add(metadata,"parents",parents);

//	char * metadata = malloc(strlen("{\"parents\":[\"0BzrbZp6jFZojc0d5TWRCcHQwWHM\"],\"title\":\"\"}")+strlen(fname));
//	sprintf(metadata,"{\"parents\":[\"0BzrbZp6jFZojc0d5TWRCcHQwWHM\"],\"title\":\"%s\"}",fname);
	buffer data;
	data.data = strdup(json_object_to_json_string(metadata));
	data.size = strlen(data.data);
	json_object_put(metadata);

	args.params = NULL;
	args.content = &data;
	args.return_headers = return_headers;

	int resp = rest_post_all(args);
	char * upload_url = return_headers[1] + strlen("Location: ");
    printf("url = '%s'\n",upload_url);
    {
        char * end = upload_url + strlen(upload_url) -1;
        while(end > upload_url && isspace(*end)) end--;
        *(end+1) = 0;
    }
	gdrive_upload(upload_url,file);
	buffer_free(data);
	i = 0;
	while (return_headers[i]!= NULL && return_headers[i][0] != '\0'){
		free(return_headers[i]);
		i++;
	}
	free(full_path);
	free(headers[0]);
	return NULL;
}

json_object * gdrive_new_folder(json_object * metadata){
	char * headers[2];
	char * bearer = ( char* ) malloc(strlen(KEY) + strlen("Bearer ")+1);
    json_object * response = NULL;
	strcpy(bearer,"Bearer "); strcat(bearer,KEY);
	rest_build_header(&headers[0],"Authorization", bearer);
	free(bearer);
	headers[1] = NULL;

	/* set up arguments */
	rest_args args;
	args.url = strdup("https://www.googleapis.com/drive/v2/files");
	args.headers = headers;
	args.params = NULL;
	args.content_type = strdup("application/json; charset=UTF-8");

	buffer data;
    buffer resp = buffer_init(0);
	data.data = strdup(json_object_to_json_string_ext(metadata, JSON_C_TO_STRING_PRETTY));
	data.size = strlen(data.data);

	args.content = &data;
    args.return_data = &resp;
	args.return_headers = NULL;

	rest_post_all(args);
    if (resp.data != NULL){
        response = json_tokener_parse(resp.data);
    }
    buffer_free(data);
    buffer_free(resp);
    return response;
}

json_object * gdrive_put_file(json_object * metadata, FILE * file){
	char * headers[2];
	char * bearer = ( char* ) malloc(strlen(KEY) + strlen("Bearer ")+1);
	char * return_headers[10];
    json_object * response = NULL;
	int i = 0;
	for (i = 0; i < 10; i ++){
		if (i == 9)return_headers[i] = "";
		else return_headers[i] = NULL;
	}
	strcpy(bearer,"Bearer "); strcat(bearer,KEY);
	rest_build_header(&headers[0],"Authorization", bearer);
	free(bearer);
//	rest_build_header(&headers[1],"X-Upload-Content-Type","application/");
//	printf("header = %s\n",headers[0]);
	headers[1] = NULL;

	/* set up arguments */
	rest_args args;
    const char * id = json_get_string(metadata, "id");
    if (id == NULL){
    	args.url = strdup("https://www.googleapis.com/upload/drive/v2/files?uploadType=resumable");
    } else {
        const char * base_url = "https://www.googleapis.com/upload/drive/v2/files/";
        args.url = (char *) malloc(strlen(base_url) + strlen(id) + strlen("?uploadType=resumable") + 1);
        strcpy(args.url, base_url);
        strcat(args.url, id);
        strcat(args.url, "?uploadType=resumable");
    }
	args.headers = headers;
	args.params = NULL;
	args.content_type = strdup("application/json; charset=UTF-8");

	buffer data;
    buffer error = buffer_init(0);
    error.size = 0;
	data.data = strdup(json_object_to_json_string_ext(metadata, JSON_C_TO_STRING_PRETTY));
	data.size = strlen(data.data);

	args.content = &data;
    args.return_data = &error;
	args.return_headers = return_headers;

	int resp = rest_post_all(args);
    printf("response = %d\n", resp);

    if (error.data == NULL){
        i=0;
        while (i < 10 && return_headers[i] != NULL){
            puts(return_headers[i++]);
        }
        char * upload_url = return_headers[1] + strlen("Location: ");
        {
            char * end = upload_url + strlen(upload_url) -1;
            while(end > upload_url && isspace(*end)) end--;
            *(end+1) = 0;
        }
        printf("url = '%s'\n",upload_url);
        response = gdrive_upload(upload_url,file);
    } else {
       printf("url: '%s'", args.url);
       printf("encountered an error\n%s\n",error.data);
       response = json_tokener_parse(error.data);
    }
	buffer_free(data);
    buffer_free(error);
	i = 0;
	while (return_headers[i]!= NULL && return_headers[i][0] != '\0'){
		free(return_headers[i]);
		i++;
	}
	free(headers[0]);
	return response;
}

json_object * gdrive_files_list(char * query, int pageToken){
	char * params[4];
	int i = 0;
	rest_build_param(&params[i++], "access_token",KEY);
	if (pageToken > 0){
		char  pageToken_string[128];
		sprintf(pageToken_string,"%d",pageToken);
		rest_build_param(&params[i++], "pageToken",pageToken_string);
	}
	if (query != NULL){
		rest_build_param(&params[i++],"q",query);
	}
	params[i] = NULL;

	buffer resp = rest_get_buffer(params,"https://www.googleapis.com/drive/v2/files");
	json_object * response = json_tokener_parse(resp.data);
	resp = buffer_free(resp);
/*	json_object * list = json_object_object_get(response,"items");
	for (i = 0; i < json_object_array_length(list); i++){
		const char * folder_id = JSON_GET_STRING(json_object_array_get_idx(list,i),"id");
		const char * title = JSON_GET_STRING(json_object_array_get_idx(list,i),"title");
		int is_dir = strcmp(JSON_GET_STRING(json_object_array_get_idx(list,i),"mimeType"),
					"application/vnd.google-apps.folder") == 0;

		printf("\"%s%s\" %*s => %s\n"
			,title
			,is_dir?"/":""
			,(int)30 - strlen(title)
			," "
			,folder_id
		);
	}*/
	i=0;
	while (params[i] != NULL){ free(params[i++]);}
	return response;
}

json_object * gdrive_folder_list(char * id, const char* pageToken, char* query){
	int i = 0;
	char * params[4];
	rest_build_param(&params[i++], "access_token",KEY);
	if (query != NULL){
		rest_build_param(&params[i++], "q", query);
	}
	if (pageToken != NULL){
		rest_build_param(&params[i++], "pageToken",pageToken);
	}
	params[i] = NULL;
	char * url = malloc(strlen("//children") + strlen("https://www.googleapis.com/drive/v2/files/")+strlen(id)+1);
	sprintf(url,"%s/%s/children","https://www.googleapis.com/drive/v2/files",id);

	buffer resp = rest_get_buffer(params,url);
	return json_tokener_parse(resp.data);
}

json_object * gdrive_files_list_children(char * id, int pageToken){
	char * params[4];
	rest_build_param(&params[0], "access_token",KEY);
//	params[1] = "maxResults=1";
	if (pageToken > 0){
		char  pageToken_string[128];
		sprintf(pageToken_string,"%d",pageToken);
		rest_build_param(&params[2], "pageToken",pageToken_string);
	} else {
		params[2] = NULL;
	}
	params[3] = NULL;
	char * url = malloc(strlen("//children") + strlen("https://www.googleapis.com/drive/v2/files/")+strlen(id)+1);
	sprintf(url,"%s/%s/children","https://www.googleapis.com/drive/v2/files",id);

	buffer resp = rest_get_buffer(params,url);
	json_object * response = json_tokener_parse(resp.data);
	json_object * list = json_object_object_get(response, "items");
	int i;
	for (i = 0; i < json_object_array_length(list); i++){
		const char * folder_id = JSON_GET_STRING(json_object_array_get_idx(list,i),"id");
		if (folder_id != NULL){
			json_object * metadata = gdrive_get_metadata(folder_id);
			printf ("%s%s => %s\n",
				JSON_GET_STRING(metadata,"title"),
				(strcmp
					(JSON_GET_STRING(metadata,"mimeType"),
					"application/vnd.google-apps.folder") == 0
				)?"/":"",
				folder_id
			);
//			json_object_put(metadata);
		}
	}

	resp = buffer_free(resp);
	free(params[0]);
	free(url);
	//free(params[1]);
	//free(params[2]);
	return response;
	//json_object_put(response);
}

json_object * gdrive_authorize_token (char * token,const char * cid,const char *csec){
	char * params[6];
	rest_build_param(&params[0],"code",token);
	rest_build_param(&params[1],"client_id",cid);
	rest_build_param(&params[2],"client_secret",csec);
	params[3] = "redirect_uri=urn:ietf:wg:oauth:2.0:oob";
	params[4] = "grant_type=authorization_code";
	params[5] = NULL;

	printf("doing post\n");
	buffer resp = rest_post(params,"https://accounts.google.com/o/oauth2/token");
	json_object * response = NULL;
	if (resp.data != NULL) response = json_tokener_parse(resp.data);

//	char * access_token = strdup(JSON_GET_STRING(response,"access_token"));

	buffer_free(resp);
	free(params[0]);
	free(params[1]);
	free(params[2]);
	return response;
}

char * gdrive_refresh_token (char * refresh_token){
	char * params[5];
	rest_build_param(&params[0],"refresh_token",refresh_token);
	rest_build_param(&params[1],"client_id",__GLOBAL_CLIENT_ID);
	rest_build_param(&params[2],"client_secret",__GLOBAL_CLIENT_SECRET);
	params[3] = "grant_type=refresh_token";
	params[4] = NULL;

	printf("doing post\n");
	buffer resp = rest_post(params,"https://accounts.google.com/o/oauth2/token");
	json_object * response = json_tokener_parse(resp.data);

	char * access_token = strdup(JSON_GET_STRING(response,"access_token"));

	json_object_put(response);
	buffer_free(resp);
	free(params[0]);
	free(params[1]);
	free(params[2]);
	return access_token;
}

const char * gdrive_access_token(const char * val){
	if (val != NULL){
		free(KEY);
		KEY = strdup(val);
	}
	return KEY;
}

void gdrive_init(const char * token, json_object * init){
	curl_global_init(CURL_GLOBAL_DEFAULT);
	KEY = strdup(token);
	cache = init;
}

void gdrive_cleanup(){
	free(KEY);
	curl_global_cleanup();
}


/*int main(int argc, char ** argv){
	cache = json_object_from_file("./cache.json");
	if (cache == NULL) cache = json_object_new_object();
//	printf("%s\n",json_object_to_json_string_ext(cache,JSON_C_TO_STRING_PRETTY));
	curl_global_init(CURL_GLOBAL_DEFAULT);
	KEY = gdrive_refresh_token(REFRESH_TOKEN);
//	gdrive_files_list("anything at all",0); */
	/*FILE * test = gdrive_get("0BzrbZp6jFZojMmctZXQwUFNOYlE");
	FILE * fout = fopen("test.zip","wb");
	char buffer[4048];
	int out=0, in,t;
	do {
		in = fread(buffer,1,4048,test);
		t+= in;
		printf("\rread %d bytes",t);fflush(stdout);
		out += fwrite(buffer,1,in,fout);
	} while (in > 0);
	printf("\n");
	fclose(test);
	fclose(fout);
	printf("wrote %d bytes to \"test.zip\"\n",out);*/

/*	FILE * test = fopen(argv[1],"r");
	printf("sending file '%s'\n",argv[1]);
	gdrive_files_put(argv[2],test);
	fclose(test);

	json_object_put(gdrive_files_list("title='test' and mimeType = 'application/vnd.google-apps.folder'",0));*/
/*	char * next_page_token = NULL;
	do {
		json_object * changes = gdrive_get_changes(next_page_token,100);
		free(next_page_token);
		next_page_token = JSON_GET_STRING(changes,"nextPageToken");
		if (next_page_token != NULL) next_page_token = strdup(next_page_token);
		int i;
		json_object * items;
		if (json_object_object_get_ex(changes, "items",&items)){
			for ( i = 0; i < json_object_array_length(items);i++){
				json_object * change = json_object_array_get_idx(items,i);
				const char * id    = JSON_GET_STRING(change,"fileId");
				if (
					!JSON_GET_BOOL(change,"deleted",false) &&
					json_object_object_get_ex(change,"file",&change)
				){
					const char * title = JSON_GET_STRING(change,"title");
					const char * modified = JSON_GET_STRING(change,"modifiedDate");
					bool is_create = strcmp(
						JSON_GET_STRING(change,"createdDate"),
						modified
					) == 0;
					bool is_dir =  strcmp(
						JSON_GET_STRING(change,"mimeType"),
						"application/vnd.google-apps.folder"
					) == 0;
					printf("file id: '%s', title: '%s%s' %s on %s\n",
						id,
						title,
						is_dir?"/":"",
						is_create?"created":"modified",
						modified
					);
				} else {
					printf("file id: '%s, deleted\n",id);
				}
			}
		}
		json_object_put(changes);
	} while (next_page_token != NULL);
	curl_global_cleanup();
	free(KEY);
//	printf("%s\n",json_object_to_json_string_ext(cache,JSON_C_TO_STRING_PRETTY));
	json_object_to_file("./cache.json",cache);
	json_object_put(cache);
}*/
