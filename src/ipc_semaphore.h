#ifndef __SYNCD_SEMAPHORE__
#define __SYNCD_SEMAPHORE__

#ifdef _WINDOWS_
#include "ipc_semaphore_windows.h"
#else
#include "ipc_semaphore_posix.h"
#endif

// this is for getting shared memory semaphores.
semaphore semaphore_create(unsigned int max);
int semaphore_delete(semaphore s);
int semaphore_wait(semaphore s);
int semaphore_post(semaphore s);

#endif
