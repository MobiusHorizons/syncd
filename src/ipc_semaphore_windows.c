#define _WINDOWS_
#include <windows.h>
#include "ipc_semaphore.h"
#include "shared_memory.h"

semaphore semaphore_create(unsigned int max){
    semaphore s;
    s.semaphore = (HANDLE) shared_mem_alloc(sizeof(HANDLE));
    s.semaphore = CreateSemaphore(NULL,max, max, NULL);
    s.max = max;
    return s;
}

int semaphore_wait(semaphore s){
    return WaitForSingleObject(s.semaphore,0);
}

int semaphore_post(semaphore s){
    return ReleaseSemaphore(s.semaphore, 1, NULL);
}

void semaphore_destroy(semaphore s){
    CloseHandle(s.semaphore);
    shared_mem_dealloc(s.semaphore, sizeof(HANDLE));
}

