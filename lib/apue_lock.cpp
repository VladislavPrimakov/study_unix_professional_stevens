#include "apue.h"

int lockfile(int fd) {
	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	return(fcntl(fd, F_SETLK, &fl));
}

int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len) {
	struct flock lock;
	lock.l_type = type; /* F_RDLCK, F_WRLCK, F_UNLCK */
	lock.l_start = offset;
	lock.l_whence = whence; /* SEEK_SET, SEEK_CUR, SEEK_END */
	lock.l_len = len;
	return(fcntl(fd, cmd, &lock));
}

pid_t lock_test(int fd, int type, off_t offset, int whence, off_t len) {
	struct flock lock;
	lock.l_type = type;
	lock.l_start = offset;
	lock.l_whence = whence;
	lock.l_len = len;
	if (fcntl(fd, F_GETLK, &lock) < 0)
		err_sys("fcntl error");
	if (lock.l_type == F_UNLCK)
		return(0);
	return(lock.l_pid);
}
