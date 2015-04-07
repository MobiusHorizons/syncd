#include <semaphore.h>

typedef struct {
    int max;
    sem_t * semaphore;
} semaphore;
