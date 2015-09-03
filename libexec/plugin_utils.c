/** plugin utilities ***/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

/** NAMESPACE plugin_ **/
#define BUF_SIZE 8192

typedef int(*plugin_pipe_cb)(size_t len, char * data, void* user_data);

typedef struct{
	plugin_pipe_cb cb;
	void* user_data;
	int in_pipe;
	int out_pipe;
} plugin_pipe_args;

void* plugin_pipe_run(void* user_def){
	plugin_pipe_args * args = (plugin_pipe_args*)user_def;
	char buf[BUF_SIZE];
	size_t total = 0;
	size_t len;
	while((len = read(args->in_pipe, buf, BUF_SIZE) > 0)){
		total += args->cb(len, buf, args->user_data);
		write(args->out_pipe,buf, len);
	}
	return NULL;
}

int plugin_pipe(pthread_t* in_thread, int in_pipe, plugin_pipe_cb cb, void* user_data){
	int pipes[2];
	int success;
	if ((success = pipe(pipes))){
		//initialize args

		plugin_pipe_args * args = malloc(sizeof(plugin_pipe_args));
		args->cb = cb;
		args->in_pipe = in_pipe;
		args->out_pipe = pipes[1];
		args->user_data = user_data;


		//pthread_t thread ; //Todo: check for null.
		success = pthread_create(in_thread, NULL, plugin_pipe_run, (void*) args);
		if (success){ // not
			return pipes[0];
		}
		return success;
	}
	return success;
}
