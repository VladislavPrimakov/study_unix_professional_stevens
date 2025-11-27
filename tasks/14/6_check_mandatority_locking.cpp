#include "apue.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
	int fd;
	pid_t pid;
	char buf[5];
	struct stat statbuf;
	if (argc != 2) {
		err_quit("Usage {} filename", argv[0]);
	}
	if ((fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, FILE_MODE)) < 0)
		err_sys("call open {}", argv[1]);
	if (write(fd, "abcdef", 6) != 6)
		err_sys("call write");
	if (fstat(fd, &statbuf) < 0)
		err_sys("call fstat");
	// set mandatory locking
	if (fchmod(fd, (statbuf.st_mode & ~S_IXGRP) | S_ISGID) < 0)
		err_sys("call fchmod");
	TELL_WAIT();
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid > 0) {
		// set lock for whole file
		if (write_lock(fd, 0, SEEK_SET, 0) < 0)
			err_sys("call write_lock");
		TELL_CHILD(pid);
		if (waitpid(pid, NULL, 0) < 0)
			err_sys("call waitpid");
	}
	else {
		WAIT_PARENT();
		set_fl(fd, O_NONBLOCK);
		if (read_lock(fd, 0, SEEK_SET, 0) != -1)
			err_sys("child: call read_lock success");
		std::println("read_lock for bytes range return code: {}", errno);
		/* теперь попробуем читать из файла под принудительной блокировкой */
		if (lseek(fd, 0, SEEK_SET) == -1)
			err_sys("call lseek");
		if (read(fd, buf, 2) < 0)
			err_ret("call read (mandatory lock works)");
		else
			std::println("data was read (mandatory lock doesn't work), buf = {}", buf);
	}
	TELL_DONE();
	exit(0);
}