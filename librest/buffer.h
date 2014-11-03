#ifndef __BUFFER_H_
#define __BUFFER_H_
#include <stdlib.h>
#ifdef WIN32
#define EXPORT_SHARED __declspec(dllexport)
#else
#define EXPORT_SHARED 
#endif
typedef struct{
	char * data;
	size_t size;
} buffer;

buffer EXPORT_SHARED buffer_init(buffer b, size_t size);
buffer EXPORT_SHARED buffer_append(buffer b, char* data, size_t length);
buffer EXPORT_SHARED buffer_clone(buffer b);
int    EXPORT_SHARED buffer_read(buffer *b, char *data, size_t length);
buffer EXPORT_SHARED buffer_from_string(char * string);
buffer EXPORT_SHARED buffer_free(buffer b);
#endif
