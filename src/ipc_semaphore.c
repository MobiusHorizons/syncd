#include "ipc_semaphore.h"
#include "shared_memory.h"

#ifdef _WINDOWS_

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

#else

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

#endif
