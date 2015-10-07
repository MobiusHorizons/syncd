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
#include "rest.h"


#ifdef WIN32
	#include <io.h>
	#include <fcntl.h>
	#define pipe(fds) _pipe(fds,4096, _O_BINARY)
	#define SSL_CERT char SSL_CA_PATH[PATH_MAX];    \
	 GetModuleFileName(NULL, SSL_CA_PATH, PATH_MAX);\
	 dirname(SSL_CA_PATH);            \
	 strcat(SSL_CA_PATH,"\\ca-bundle.crt");         \
	 curl_easy_setopt(curl, CURLOPT_CAINFO, SSL_CA_PATH); printf("SSL_CA_PATH = '%s'\n",SSL_CA_PATH);
#else
	#define SSL_CERT
//	#define SSL_CERT curl_easy_setopt(curl, CURLOPT_CAPATH, "./ca-bundle.crt");
#endif
pthread_t threads[256];

int rest_build_param(char** param, const char * name, const char* value){
	*param = (char*)malloc(strlen(name)+strlen(value) + 2);
	return sprintf(*param, "%s=%s", name, value);
}

int rest_build_header(char ** header, const char * name, const char * value){
	*header= (char*)malloc(strlen(name) + strlen(value) + 4);
	return sprintf(*header,"%s: %s", name,value);
}

char * rest_build_url(char ** params, char* base){
	if (params == NULL) return strdup(base);
	char * url;
	int num_params;
	int i;
	int len = 0;
	for (i = 0; params[i] != NULL; i++){
		len += strlen(params[i]) + 1;
	}
	num_params = i;
	url = (char*) malloc(strlen(base) + len + 2);
	strcpy(url, base);
	strcat(url, "?");
	for ( i = 0; i < num_params; i ++){
		strcat(url, params[i]);
		if ( i< num_params-1)
			strcat(url, "&");
	}
	char *escaped = rest_escape(url);
	free(url);
	return escaped;
}

char * rest_escape(char * url){
	size_t len = strlen(url);
	size_t diff = 0;
	size_t current = len;
	int i;
	char * escaped = (char*) malloc(len + 1);
	escaped[0] = '\0';
	for ( i = 0; i < len; i ++){
		if (len+diff +3> current){
			current += 4;
			escaped = (char*)realloc(escaped,current+1);
		}
		char c = url[i];
		switch (c){
		case ' ':
			diff += 2;
			strcat(escaped,"%20");
			break;
		default:
			escaped[i+diff]=c;
			escaped[i+diff+1] = '\0';
			break;
		}
	}
	return escaped;
}


void * run_curl(void* ptr){
	if (ptr != NULL){
		run_curl_args * args = (run_curl_args*) ptr;
		CURLcode res = curl_easy_perform(args->curl);
	  if (res != CURLE_OK){
			 printf("rest_get error: %s\n", curl_easy_strerror(res));
		}
		fclose(args->file);
		free(args->url);
		curl_easy_cleanup(args->curl);
	}
    return NULL;
}

size_t ReadFileCB( void *contents, size_t size, size_t nmemb, void *userp){
	FILE * file = (FILE*) userp;
	if (file == NULL) file = stdin;
	return fread(contents,size,nmemb,file);
}

size_t ReadBufferCB( void *contents, size_t size, size_t nmemb, void *userp){
	if (userp != NULL){
		size_t realsize = size * nmemb;
		buffer * data = (buffer*) userp;
		realsize = buffer_read(data,contents,realsize);
		//printf("size = %d,contents = %.*s\n",(int)realsize,(int)realsize,(char*)contents);
		return realsize;
	}
    return 0;
}

static size_t WriteFileCB(void * contents, size_t size, size_t nmemb, void * userp){
	FILE * file = (FILE*) userp;
	if (file == NULL) file = stdout;
	return fwrite(contents, size, nmemb, file);
}

static size_t WriteBufferCB(void *contents, size_t size, size_t nmemb, void *userp)
{
	if (userp!= NULL){
		size_t realsize = size * nmemb;
		buffer * data = (buffer*) userp;
        if (realsize > 0){
		    *data = buffer_append(*data,contents, realsize);
        }
		return realsize;
	}
    return 0;
}

static size_t WriteLinesCB(void *contents, size_t size, size_t nmemb, void *userp)
{
	if (userp!= NULL){
		int i = 0;
		char ** lines = (char**) userp;
		while (lines[i] != NULL && lines[i][0] != '\0' /*last entry*/) i++;
		// i now equals the first writable line if lines[i] = NULL
		if (lines[i] == NULL && ((char*)contents)[0] != '\n'){
			size_t realsize = size * nmemb;
			lines[i] = malloc(realsize);
			memcpy(lines[i],contents,realsize);
			lines[i][realsize-1] = '\0'; // cut the newline
			return realsize;
		} else {
			return size*nmemb; // ignore
		}
	}
    return 0;
}

FILE * rest_get	(char ** params, char * url){
	char * full_url = rest_build_url(params, url);

	/* set up pipes for reading/writing*/
	int fd[2];
	if (pipe(fd)!=0){ // uh oh, we have something wrong
	//	printf("could not create pipe\n");
		exit(1);
	}
	FILE * read = fdopen(fd[0], "r");
	FILE * write = fdopen(fd[1], "w");
	CURL * curl = curl_easy_init();
	SSL_CERT
//	printf("url = %s\n",full_url);
	curl_easy_setopt(curl,CURLOPT_URL,full_url);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,(void*)write);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,WriteFileCB);
	run_curl_args *args = malloc(sizeof(run_curl_args));
	args->curl = curl;
	args->file = write;
	args->url = full_url;
	pthread_create(&threads[fd[1]],NULL, run_curl, args);
	return read;
}

buffer rest_get_buffer (char ** params, char * url){
	char * full_url = rest_build_url(params, url);
	CURL * curl = curl_easy_init();
	SSL_CERT
	buffer data = buffer_init(0);
	curl_easy_setopt(curl,CURLOPT_URL,full_url);
//	printf("url = %s\n",full_url);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,&data);
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,WriteBufferCB);
	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK){
		//TODO: return better error handling;
	 	 printf("rest_get_buffer error: %s\n", curl_easy_strerror(res));
		//printf("code = %d, data= %s\n",res,data.data);
	}
	free(full_url);
	curl_easy_cleanup(curl);
	return data;
}

buffer  rest_post (char ** params, char * url){
	CURL * curl = curl_easy_init();
	char * post = rest_build_url(params,"");
	char * escaped_url = rest_escape(url);

	SSL_CERT
	buffer data = buffer_init(0);
	curl_easy_setopt(curl,CURLOPT_URL,escaped_url);
	curl_easy_setopt(curl,CURLOPT_POSTFIELDS,post+1);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA, &data);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,WriteBufferCB);
	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK){
			 printf("rest_post error: %s\n", curl_easy_strerror(res));
	}
	free(post);
	free(escaped_url);
	curl_easy_cleanup(curl);
	return data;
}

buffer  rest_post_headers (char ** params, char ** headers, char * url){
	CURL * curl = curl_easy_init();
	char * post = rest_build_url(params,""); post++;
	struct curl_slist * slist = NULL;
	int i = 0;
        while (headers[i] != NULL){
		slist = curl_slist_append(slist,headers[i++]);
	}
	char * escaped_url = rest_escape(url);
	SSL_CERT
	buffer data = buffer_init(0);
	curl_easy_setopt(curl,CURLOPT_URL,escaped_url);
	curl_easy_setopt(curl,CURLOPT_POSTFIELDS,post);
	curl_easy_setopt(curl,CURLOPT_HTTPHEADER, slist);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA, &data);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,WriteBufferCB);
	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK){
		//TODO: better error handling
			 printf("rest_post_headers error: %s\n", curl_easy_strerror(res));
	}
	post++;
	free(post);
	free(escaped_url);
	curl_slist_free_all(slist);
	curl_easy_cleanup(curl);
	return data;
}

int rest_post_all(rest_args args){
	CURL * curl = curl_easy_init();
	char * post = NULL;
	char * escaped_url = NULL;
	char content_length[100];
	char * content_type = NULL;
	struct curl_slist *slist = NULL;
	int i;
	SSL_CERT
	if (args.params != NULL){
		int len = 0;
		int n = 0;
		while (args.params[n] != NULL)  len += strlen(args.params[n++]) + 1;
        if (len > 0){
            post = malloc(len);
            post[0] = '\0';
            for (i = 0; i < n; i++){
                strcat(post,args.params[i]);
                if (i < n-1) strcat(post,"&");
            }
            curl_easy_setopt(curl,CURLOPT_POSTFIELDS,post);
        }
	}
	if (args.content != NULL){
		curl_easy_setopt(curl,CURLOPT_POSTFIELDS,args.content->data);
		rest_build_header(&content_type,"Content-Type",args.content_type);
		sprintf(content_length,"Content-Length: %d",(int)args.content->size);
		slist = curl_slist_append(slist,content_type);
		slist = curl_slist_append(slist,content_length);
	}
	if (args.headers !=NULL){
		i = 0;
		while (args.headers[i] != NULL){
			slist = curl_slist_append(slist,args.headers[i++]);
		}
		curl_easy_setopt(curl,CURLOPT_HTTPHEADER,slist);
    }
	if (args.return_headers != NULL){
		curl_easy_setopt(curl,CURLOPT_WRITEHEADER,args.return_headers);
		curl_easy_setopt(curl,CURLOPT_HEADERFUNCTION,WriteLinesCB);
	}
	if (args.url != NULL){
		escaped_url = rest_escape(args.url);
		curl_easy_setopt(curl,CURLOPT_URL,escaped_url);
	}

	if (args.return_data != NULL){
		curl_easy_setopt(curl,CURLOPT_WRITEDATA, args.return_data);
		curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,WriteBufferCB);
	}
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK){
	 	 printf("rest_post_all error: %s\n", curl_easy_strerror(res));
	}
	free(post);
	free(content_type);
	free(escaped_url);
	curl_slist_free_all(slist);
	curl_easy_cleanup(curl);
	return res;
}

int rest_put_all(rest_args args, FILE * file){
	CURL * curl = curl_easy_init();
	char * full_url = NULL;
	char content_length[100];
	char * content_type = NULL;
	struct curl_slist *slist = NULL;
	int i;
	SSL_CERT
    if (args.params != NULL){
        full_url = rest_build_url(args.params,args.url);
	} else if (args.url != NULL){
        full_url = strdup(args.url);
    }
	if (args.content != NULL){
        curl_easy_setopt(curl,CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(curl,CURLOPT_READDATA,args.content);
        curl_easy_setopt(curl,CURLOPT_READFUNCTION, ReadBufferCB);
		rest_build_header(&content_type,"Content-Type",args.content_type);
		sprintf(content_length,"Content-Length: %d",(int)args.content->size);
		slist = curl_slist_append(slist,content_type);
		slist = curl_slist_append(slist,content_length);
	}
	if (args.headers !=NULL){
		i = 0;
		while (args.headers[i] != NULL){
			slist = curl_slist_append(slist,args.headers[i++]);
		}
		curl_easy_setopt(curl,CURLOPT_HTTPHEADER,slist);
	}
	if (args.return_headers != NULL){
		curl_easy_setopt(curl,CURLOPT_WRITEHEADER,args.return_headers);
		curl_easy_setopt(curl,CURLOPT_HEADERFUNCTION,WriteLinesCB);
	}
	if (args.url != NULL){
		curl_easy_setopt(curl,CURLOPT_URL,full_url);
	}
	if (file != NULL){
        curl_easy_setopt(curl,CURLOPT_UPLOAD, 1L);
    	curl_easy_setopt(curl,CURLOPT_READDATA,file);
        curl_easy_setopt(curl,CURLOPT_READFUNCTION,ReadFileCB); // for windows

    }

	if (args.return_data != NULL){
		curl_easy_setopt(curl,CURLOPT_WRITEDATA, args.return_data);
		curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,WriteBufferCB);
	}

	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK){
	 printf("rest_post_all error: %s\n", curl_easy_strerror(res));
	}
	free(content_type);
	free(full_url);
	curl_slist_free_all(slist);
	curl_easy_cleanup(curl);
	return res;
}

buffer rest_put_file (char** params, char* url, FILE * in){
	CURL * curl = curl_easy_init();
	SSL_CERT
	buffer data = buffer_init(0);
	char * full_url = rest_build_url(params,url);
//	printf("%s\n",full_url);
	curl_easy_setopt(curl,CURLOPT_URL,full_url);
	curl_easy_setopt(curl,CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(curl,CURLOPT_READDATA,in);
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,&data);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,WriteBufferCB);
	curl_easy_setopt(curl,CURLOPT_READFUNCTION,ReadFileCB); // for windows
	CURLcode res = curl_easy_perform(curl);
	free(full_url);
	if (res != CURLE_OK){
		//printf("res = %d, data = %.*s\n",res,(int)data.size,data.data);
	  printf("rest_put_file error: %s\n", curl_easy_strerror(res));
		data = buffer_free(data);
	}
	curl_easy_cleanup(curl);
	return data;
}

buffer rest_put_headers (char** params,char** headers, char* url, FILE * in){
	CURL * curl = curl_easy_init();
	SSL_CERT
	struct curl_slist *slist = NULL;
	buffer data = buffer_init(0);
	char * full_url = rest_build_url(params,url);
	int i;
//	printf("%s\n",full_url);
	if (headers != NULL){
		i = 0;
		while (headers[i] != NULL){
			slist = curl_slist_append(slist,headers[i++]);
		}
		curl_easy_setopt(curl,CURLOPT_HTTPHEADER,slist);
	}
	curl_easy_setopt(curl,CURLOPT_URL,full_url);
	curl_easy_setopt(curl,CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(curl,CURLOPT_READDATA,in);
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,&data);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,WriteBufferCB);
	curl_easy_setopt(curl,CURLOPT_READFUNCTION,ReadFileCB); // for windows
	CURLcode res = curl_easy_perform(curl);
	free(full_url);
	curl_slist_free_all(slist);
	if (res != CURLE_OK){
	  printf("rest_put_headers error: %s\n", curl_easy_strerror(res));
		//printf("res = %d, data = %.*s\n",res,(int)data.size,data.data);
		data = buffer_free(data);
	}
	curl_easy_cleanup(curl);
	return data;
}
