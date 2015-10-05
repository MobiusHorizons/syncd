#ifndef SYNC_LOCK_H

#ifdef USE_SEMAPHORE_LK
#include "ipc_semaphore.h"
#endif

/***********
 * Define Lock type. This will map to some unrelying locking mechanism
 */

enum syncd_lock_type {
  lock_shared,
  lock_exclusive
};

typedef struct {
#ifdef USE_FLOCK_LK
   int fd;
#endif
#ifdef USE_SEMAPHORE_LK
   semaphore sem;
#endif
} syncd_lock;


syncd_lock syncd_lock_create(const char * fname, int fd);

int syncd_lock_wait   (syncd_lock, enum syncd_lock_type);
int syncd_lock_release(syncd_lock, enum syncd_lock_type);

void syncd_lock_delete(syncd_lock);

#endif
