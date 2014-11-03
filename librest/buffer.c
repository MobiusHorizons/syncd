#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

buffer buffer_init(buffer b, size_t size){
/*	if (b.data != NULL && b.size != 0){
		free(b.data);
	}*/
	b.data = (char*)malloc(size);
	b.size = size;
	return b;
}

buffer buffer_append(buffer b, char* data, size_t length){
	size_t newLength = b.size + length;
	b.data = (char*) realloc(b.data, newLength+1);
	//printf("appending %d bytes ...",length);
	//fflush(stdout);
	memcpy(b.data + b.size,data, length);
	//printf("DONE\n");
	b.size = newLength;
	b.data[newLength] = '\0';
	return b;
}

int buffer_read(buffer *b, char *data, size_t length){
	length = (length > b->size)? b->size: length;
	size_t  newLength =  b->size - length;
	char * ptr =memcpy(data,b->data,length);
	char * shorter = malloc(newLength+1);
	memcpy(shorter,b->data + length,newLength);
	free(b->data);
	shorter[newLength] = '\0';
	b->data = shorter;
	b->size = newLength;
	return length;
}

buffer buffer_from_string(char * string){
	buffer b = buffer_init(b,0);
	b.data = strdup(string);
	b.size = strlen(string);
	return b;
}

buffer buffer_free(buffer b){
	free(b.data);
	b.data = NULL;
	b.size = 0;
	return b;
}

buffer buffer_clone(buffer b){
	buffer n;
	n = buffer_init(n,b.size);
	memcpy(n.data,b.data,b.size);
	return n;
}
