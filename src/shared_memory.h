#ifndef SHARED_MEM_H_
#define SHARED_MEM_H_
#include <stdlib.h>

void * shared_mem_alloc(size_t size);
void shared_mem_dealloc(void* mem, int len);

#endif
