#include "flock.h"


int flock (int fd, int operation){
	int cmd, r;
	struct flock fl;

	if (operation & LOCK_NB)
		cmd = F_SETLK;
	else
		cmd = F_SETLKW;
	operation &= ~LOCK_NB;

	memset (&fl, 0, sizeof fl);
	fl.l_whence = SEEK_SET;
	/* l_start & l_len are 0, which as a special case means "whole file". */

	switch (operation)
	{
		case LOCK_SH:
			fl.l_type = F_RDLCK;
			break;
		case LOCK_EX:
			fl.l_type = F_WRLCK;
			break;
		case LOCK_UN:
			fl.l_type = F_UNLCK;
			break;
		default:
			errno = EINVAL;
			return -1;
	}

	r = fcntl (fd, cmd, &fl);
	if (r == -1 && errno == EACCES)
		errno = EAGAIN;

	return r;
}
