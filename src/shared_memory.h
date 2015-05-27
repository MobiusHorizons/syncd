#ifndef SHARED_MEM_H_
#define SHARED_MEM_H_
#include <stdlib.h>

void * shared_mem_alloc(size_t size, const char * name);
void shared_mem_dealloc(void* mem, size_t len, const char * name);

#endif //SHARED_MEM_H_
