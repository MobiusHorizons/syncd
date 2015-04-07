#ifdef _WINDOWS_
#include "ipc_semaphore_windows.c"
#else
#include "ipc_semaphore_posix.c"
#endif
