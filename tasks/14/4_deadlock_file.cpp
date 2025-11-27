#include "apue.h"
#include <fcntl.h>

void lockabyte(const char* name, int fd, off_t offset) {
	if (writew_lock(fd, offset, SEEK_SET, 1) < 0)
		err_sys("call writew_lock {}", name);
	std::println("{}: set lock for byte {}", name, offset);
}

int main() {
	int fd;
	pid_t pid;
	if ((fd = creat("templock", FILE_MODE)) < 0)
		err_sys("call creat \"templock\"");
	if (write(fd, "ab", 2) != 2)
		err_sys("call write to \"templock\" \"ab\"");
	TELL_WAIT();
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid == 0) {
		lockabyte("child", fd, 0);
		TELL_PARENT(getppid());
		WAIT_PARENT();
		lockabyte("child", fd, 1);
	}
	else {
		lockabyte("parent", fd, 1);
		TELL_CHILD(pid);
		WAIT_CHILD();
		lockabyte("parent", fd, 0);
	}
	TELL_DONE();
	return 0;
}