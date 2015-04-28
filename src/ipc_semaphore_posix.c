#include "ipc_semaphore.h"
#include "shared_memory.h"

semaphore semaphore_create(unsigned int max){
    semaphore s;
    s.max = max;
    s.semaphore = shared_mem_alloc(sizeof(sem_t));
    sem_init(s.semaphore, 1,max);
    return s;
}

int semaphore_delete(semaphore s){
    int ret = sem_destroy(s.semaphore);
    if (ret == 0) shared_mem_dealloc(s.semaphore, sizeof(sem_t));
    return ret;
}

int semaphore_wait(semaphore s){
    return sem_wait(s.semaphore);
}

int semaphore_post(semaphore s){
    return sem_post(s.semaphore);
}
