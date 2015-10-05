#include "../build/config.h"
#include "lock.h"


#ifdef USE_FLOCK_LK
#include <sys/file.h>
#endif

#ifdef USE_SEMAPHORE_LK
#include "ipc_semaphore.h"
#endif

syncd_lock syncd_lock_create(const char * fname, int fd){
	syncd_lock lck;

	#ifdef USE_FLOCK_LK
		lck.fd = fd;
  #endif
  #ifdef USE_SEMAPHORE_LK
	  lck.sem = semaphore_create(1, fname );
  #endif
	return lck;
}


int syncd_lock_wait(syncd_lock lock, enum syncd_lock_type type){
#ifdef USE_FLOCK_LK
	int operation;
	switch (type){
	 case lock_shared:
	 	 operation = LOCK_SH;
	   break;
	 case lock_exclusive:
	   operation = LOCK_EX;
	   break;
	 default:
	   // this makes no sense.
		 return -1;
	}
	return flock(lock.fd, operation);
#endif

#ifdef USE_SEMAPHORE_LK
	return semaphore_wait(lock.sem);
#endif
	return -1;
}

int syncd_lock_release(syncd_lock lock, enum syncd_lock_type type){
#if USE_FLOCK_LK
	return flock(lock.fd, LOCK_UN);
#endif

#ifdef USE_SEMAPHORE_LK
	return semaphore_post(lock.sem);
#endif
	return -1;
}

void syncd_lock_delete(syncd_lock lock){
#ifdef USE_SEMAPHORE_LK
	semaphore_delete(lock.sem);
#endif
}
