#include <config.h>

#ifdef HAS_SYS_FILE_H
#  include <sys/file.h>
#else

#define LOCK_SH 0x0
#define LOCK_EX 0x1
#define LOCK_UN 0x2
#define LOCK_NB 0x4

int flock(int fd, int operation);
