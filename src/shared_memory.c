#include "shared_memory.h"
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void * shared_mem_alloc(size_t size){
    char name[14];
    time_t now = time(0);
    sprintf(name, "/%10ld", (long int) now % 10000000000);
    int fd = shm_open(name, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
    ftruncate(fd, size);
    void * ret = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    shm_unlink(name);
    return ret;
}

void shared_mem_dealloc(void* mem, int len){
    munmap(mem, len);
}
