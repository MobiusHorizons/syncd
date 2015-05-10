#ifndef __SYNCD_SEMAPHORE__
#define __SYNCD_SEMAPHORE__

#ifdef _WINDOWS_

	#include <windows.h>

	typedef struct {
		HANDLE semaphore;
		int max;
	} semaphore;

#else

	#include <semaphore.h>

	typedef struct {
	    int 	max;
	    sem_t * semaphore;
	} semaphore;

#endif

// this is for getting shared memory semaphores.
semaphore 	semaphore_create(unsigned int max);
int 		semaphore_delete(semaphore 		s);
int 		semaphore_wait	(semaphore 		s);
int 		semaphore_post	(semaphore 		s);

#endif
