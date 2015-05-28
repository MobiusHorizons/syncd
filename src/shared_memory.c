#include "shared_memory.h"
#include <sys/stat.h>        /* For mode constants */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef WIN32
#include <windows.h>

    typedef struct {
        HANDLE hMapFile;
        void * mem;
        char * name;
    } sm_obj;

    sm_obj * sm_mapping;
    int sm_mapping_length;

    void * shared_mem_alloc(size_t size, const char * name){
        HANDLE hMapFile;
        void * mem;
        hMapFile = CreateFileMapping(
                 INVALID_HANDLE_VALUE,    // use paging file
                 NULL,                    // default security
                 PAGE_READWRITE,          // read/write access
                 0,                       // maximum object size (high-order DWORD)
                 size,                    // maximum object size (low-order DWORD)
                 name
         );
         if (hMapFile == NULL){
             // We have a problem.
             return NULL;
         }
         mem = (void *) MapViewOfFile(hMapFile,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,
                        0,
                        size
        );

        sm_obj m;
        m.hMapFile = hMapFile;
        m.mem = mem;
        m.name = strdup(name);

        sm_mapping = realloc(sm_mapping, sizeof(sm_obj) * ++sm_mapping_length);
        sm_mapping[sm_mapping_length-1] = m;

        return (void *) mem;
    }

    void shared_mem_dealloc(void* mem, size_t len, const char * name){
        // TODO: free the sm_obj, collapsing the array.
        int i;
        sm_obj m;
        for (i = 0; i < sm_mapping_length; i++){
            m = sm_mapping[i];
            if (strcmp(m.name,name) == 0) break;
        }

        UnmapViewOfFile((LPTSTR) mem);
        CloseHandle(m.hMapFile);
        free(m.name);
        m.name = NULL;
        m.mem = NULL;
    }

#else

#include <unistd.h>
#include <fcntl.h>  /* For O_* constants */
#include <sys/mman.h>

    void * shared_mem_alloc(size_t size, const char * name){
        int fd = shm_open(name, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
        ftruncate(fd, size);
        void * ret = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        return ret;
    }

    void shared_mem_dealloc(void* mem, size_t len, const char * name){
        munmap(mem, len);
        shm_unlink(name);
    }
#endif
