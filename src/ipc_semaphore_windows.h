#ifndef _IPC_SEMAPHORE_WIN_
#define _IPC_SEMAPHORE_WIN_

#include <windows.h>

typedef struct {
    HANDLE semaphore;
    int max;
} semaphore;

#endif
