#include "ipc_semaphore.h"
#include "shared_memory.h"

#ifdef WIN32

	semaphore semaphore_create(unsigned int max, const char * name){
		  semaphore s;
		  s.semaphore = CreateSemaphore(NULL, max, max, name);
		  s.max = max;
		  return s;
	}

	int semaphore_wait(semaphore s){
		  return WaitForSingleObject(s.semaphore,0);
	}

	int semaphore_post(semaphore s){
		  return ReleaseSemaphore(s.semaphore, 1, NULL);
	}

	int semaphore_delete(semaphore s, const char * name){
		  CloseHandle(s.semaphore);
		  return 1;
	}

#else

	semaphore semaphore_create(unsigned int max, const char * name){
		  semaphore s;
		  s.max = max;
		  s.semaphore = shared_mem_alloc(sizeof(sem_t), name);
		  sem_init(s.semaphore, 1,max);
		  return s;
	}

	int semaphore_delete(semaphore s, const char *name){
		  int ret = sem_destroy(s.semaphore, name);
		  if (ret == 0) shared_mem_dealloc(s.semaphore, sizeof(sem_t));
		  return ret;
	}

	int semaphore_wait(semaphore s){
		  return sem_wait(s.semaphore);
	}

	int semaphore_post(semaphore s){
		  return sem_post(s.semaphore);
	}

#endif
