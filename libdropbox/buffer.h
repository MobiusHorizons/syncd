#ifndef __BUFFER_H_
#define __BUFFER_H_
#include <stdlib.h>
#include <string.h>

typedef struct{
	char * data;
	size_t size;
} buffer;

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
#endif
