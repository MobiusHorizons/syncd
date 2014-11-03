#include "dropbox_api.h"
#include <string.h>
#include <stdbool.h>

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

const char * init(char * client_key,char * client_secret){
	FILE * state = fopen("access_token.txt", "r");
	const char * access_token = NULL;
	if (state != NULL){
		static char at[128];
		fgets(at, 128, state);
		if (at != NULL){
			int len = strlen(at);
			at[len-1] = '\0';
			access_token = at;
		}
		fclose(state);
	}
	if (access_token == NULL){
		char  token[128];
		char cmd[256];
		#ifdef WIN32
			#define URL_OPEN_CMD "start \"\""
		#else
			#define URL_OPEN_CMD "xdg-open"
		#endif
		sprintf(cmd,"%s \"%s?response_type=code&client_id=%s\"",URL_OPEN_CMD,OAUTH_2_AUTH,client_key);
		system(cmd);
		printf("paste code here\n");
		if (fgets(token,128,stdin) == NULL) exit(1);
		int len = strlen(token);
		if (token[len-1] == '\n') token[len-1] = '\0';
		access_token = db_authorize_token(token,client_key,client_secret);
		FILE * state = fopen("access_token.txt","w");
		fprintf(state ,"%s\n",access_token);
		fclose(state);
	}
	if (access_token == NULL) exit(-1);
	return access_token;
}

void do_put(char* in, char* out, const char* access_token){
	printf("doing put %s %s\n",in,out);
	FILE * file = fopen(in,"r");
	if (file == NULL){
		printf("could not open file '%s'\n",in);	
		exit(-1);
	}
	json_object * upload = db_files_put(out,access_token,file);
	if (json_object_object_get_ex(upload,"error",NULL)){
		printf("%s\n", JSON_GET_STRING(upload,"error"));
	} else {
		printf("uploaded '%s' to '%s'\n",in,JSON_GET_STRING(upload,"path"));
	}
	fclose(file);
	json_object_put(upload);
}

void do_ls(char* file, const char *access_token){
	int i;
	json_object * metadata = db_metadata(file,access_token,true);
	json_object * contents;
	if (json_object_object_get_ex(metadata,"error",NULL)){
		printf("%s\n", JSON_GET_STRING(metadata,"error"));
	} else if (json_object_object_get_ex(metadata,"contents",&contents)){
		for ( i = 0; i < json_object_array_length(contents); i ++){
			json_object * value = json_object_array_get_idx(contents,i);
			json_object * path;
			if (json_object_object_get_ex(value,"path",&path)){
				printf("%s%s\n",json_object_get_string(path),JSON_GET_BOOL(value,"is_dir",false)?"/":"");
			}
		}
	}
	json_object_put(metadata);
}

void do_metadata(char * file, const char*access_token){
	json_object * resp = db_metadata(file,access_token,true);
	if (json_object_object_get_ex(resp,"error",NULL)){
		printf("%s\n", JSON_GET_STRING(resp,"error"));
	} else {
		printf("%s\n", json_object_to_json_string(resp));
	}
	json_object_put(resp);
}

void do_rm(char * file, const char*access_token){
	json_object * resp = db_rm(file,access_token);
	if (JSON_GET_BOOL(resp,"is_deleted",false)){
		printf("Deleted %s from dropbox\n", JSON_GET_STRING(resp,"path"));
	} else {
		printf("%s\n", JSON_GET_STRING(resp,"error"));
	}
	json_object_put(resp);
}

void do_mv(char * file1, char * file2, const char*access_token){
	json_object * resp = db_mv(file1,file2,access_token);
	if (json_object_object_get_ex(resp,"error",NULL)){
		printf("%s\n", JSON_GET_STRING(resp,"error"));
	} else {
		printf("Moved file %s to %s\n",file1,JSON_GET_STRING(resp,"path"));
	}
	json_object_put(resp);
}

void do_mkdir(char * file, const char*access_token){
	json_object * resp = db_mkdir(file,access_token);
	if (json_object_object_get_ex(resp,"error",NULL)){
		printf("%s\n", JSON_GET_STRING(resp,"error"));
	} else {
		printf("Created %s/\n", JSON_GET_STRING(resp,"path"));
	}
	json_object_put(resp);
}

void do_get(char * file, const char * access_token){
		FILE * out_file = db_files_get(file,access_token);
		char  resp[1024];
		int i;
		do {
			i = fread(resp, 1, 1024, out_file);
			printf("%.*s", i,resp);
		} while(i);
		fclose(out_file);
}

int main(int argc, char** argv){
	char * client_key    = "gmq6fs74fuw1ead";
	char * client_secret = "ia87pt0ep6dvb7y";
	curl_global_init(CURL_GLOBAL_DEFAULT);
	const char * access_token = init(client_key,client_secret);
//	printf("access_token = %s\n",access_token);
	if (argc >= 2 && strcmp(argv[1],"ls")== 0 ){
		if (argc == 2) do_ls("/",access_token);
		else do_ls(argv[2],access_token);
	} else if (argc >=2 && strcmp(argv[1],"metadata")==0){
		do_metadata(argv[2],access_token);
	} else if (argc == 4 && strcmp(argv[1],"put")==0){
		do_put(argv[2],argv[3],access_token);
	} else if (argc == 3 && strcmp(argv[1],"get")==0){
		do_get(argv[2],access_token);
	} else if (argc == 3 && strcmp(argv[1],"rm")==0){
		do_rm(argv[2],access_token);
	} else if (argc == 4 && strcmp(argv[1],"mv")==0){
		do_mv(argv[2],argv[3],access_token);
	} else if (argc == 3 && strcmp(argv[1],"mkdir")==0){
		do_mkdir(argv[2],access_token);
	} else {
		printf("usage dropbox [command] [args]\n"
			"commands:\n"
			"\tls [dir]\n"
			"\tmetadata [file/dir]\n"
			"\tput [local] [remote]\n"
			"\tget [remote]\n"
			"\trm  [remote]\n"
			"\tmv  [old_name] [new_name]\n"
			"\tmkdir  [target] \n"
		);
	}

/*	json_object * delta = db_delta(NULL,access_token);
	const char * cursor;
	if (json_object_object_get_ex(delta,"cursor",&temp_obj)){
		cursor = json_object_get_string(temp_obj);
	}
	printf("cursor = %s\n",cursor);*/
	curl_global_cleanup();
}
