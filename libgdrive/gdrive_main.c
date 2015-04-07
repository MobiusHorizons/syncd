#include "gdrive_api.h"
#define REFRESH_TOKEN "1/8obRmFxvhhebWSCYckmw_AfUlfTD-ERnwvoro8tMAKI" 

int main(int argc, char ** argv){
	json_object * cache = json_object_from_file("./cache.json");
	if (cache == NULL) cache = json_object_new_object();
//	printf("%s\n",json_object_to_json_string_ext(cache,JSON_C_TO_STRING_PRETTY));
	curl_global_init(CURL_GLOBAL_DEFAULT);
	gdrive_init(gdrive_refresh_token(REFRESH_TOKEN),cache);
//	gdrive_files_list("anything at all",0);
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
	char * next_page_token = NULL;
	do {
		json_object * changes = gdrive_get_changes(next_page_token,0,10);
//		printf("%s\n",json_object_to_json_string_ext(changes,JSON_C_TO_STRING_PRETTY));
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
	gdrive_cleanup();
//	printf("%s\n",json_object_to_json_string_ext(cache,JSON_C_TO_STRING_PRETTY));
	json_object_to_file("./cache.json",cache);
	json_object_put(cache);
}
