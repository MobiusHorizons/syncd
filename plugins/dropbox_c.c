#include "Python.h"
#include "dropbox_py.h"
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#define PLUGIN_PREFIX "dropbox://"
#define PLUGIN_PREFIX_LEN 10 // i counted

FILE * sync_open(char*,const char*);
char * init(){
	PyEval_InitThreads();
	Py_Initialize();
	initdropbox_py();
	//FILE* f = fopen("/home/paul/src/syncronization/state.json","rb");
	//py_write("/test.txt",f);
	return PLUGIN_PREFIX;
}

void unload(){
	Py_Finalize();
}

/*int cb (char * path, int mask){
	printf("path = %s, mask = %d\n",path,mask);
}*/

void listen(int (*cb)(char*,int)){
	PyEval_InitThreads();
	longpoll(cb);
}

void do_poll(int interval){
	PyEval_InitThreads();
	printf("sleeping %d seconds\n",interval-(time(NULL)%interval));
	sleep (interval - (time(NULL)%interval));
	update();
}


void watch_dir(char*path){
printf("i am supposed to be monitoring directory %s\n",path);
}

FILE * sync_open(char*path,const char* specifier){
	PyEval_InitThreads();
	path += PLUGIN_PREFIX_LEN;
	char * fn = py_open(path);
	FILE * fp = fopen(fn,specifier);
	//FILE * fp = fdopen(fd,specifier);
  //      char data[1025];
  //      fread(data,1024,sizeof(char),fp);
  //      printf("read %s\n",data);
	return fp;
}

void sync_close(FILE *fp){
        printf("closing file ... "); fflush(stdout);
	char fname[50];
	sprintf(fname,"/tmp/%d-%d",getpid(),fileno(fp));
	printf(fname);
	fclose(fp);
	unlink(fname);
	printf("SUCCESS\n");
}

int sync_read (int id, char * buffer, int len){
	PyEval_InitThreads();
	return 0;
}

int sync_write(char * path, FILE * fp){
	PyEval_InitThreads();
	printf("path = '%s', file = %p\n",path,fp);
	path += PLUGIN_PREFIX_LEN;
	return py_write(path,fp);
}

int sync_mkdir(char* path){
	return 0;
}	

int sync_rm(char * path){
	return 0;
}

int sync_mv(char * from, char* to){
	return 0;
}

char * curl_get(char*cmd){
	PyEval_InitThreads();
	printf("curl command : %s\n");
	char * response = malloc(1025);
	char * part = response;
	FILE * curl = popen(cmd,"r");
	int i,size;
	while ((i = fread(part,1,1024,curl) > 0)){
		size += i;
		response = realloc(response,size+1025);
		part = &response[size];
	}
	pclose(curl);
	return response;
}
