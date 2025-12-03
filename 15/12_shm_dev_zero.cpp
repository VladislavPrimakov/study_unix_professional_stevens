#include "apue.h"
#include <sys/mman.h>

#define NLOOPS 1000
#define SIZE sizeof(long) // size of shared memory area

int update(long* ptr) {
	return((*ptr)++); // return value before increment
}

int main() {
	int fd, i, counter;
	pid_t pid;
	void* area;
	if ((fd = open("/dev/zero", O_RDWR)) < 0)
		err_sys("call open /dev/zero");
	if ((area = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
		err_sys("call mmap");
	close(fd);
	TELL_WAIT_SIGNAL();
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid > 0) { // parent
		for (i = 0; i < NLOOPS; i += 2) {
			if ((counter = update((long*)area)) != i)
				err_quit("parent: expected {}, taken {}", i, counter);
			TELL_CHILD_SIGNAL(pid);
			WAIT_CHILD_SIGNAL();
		}
	}
	else { // child
		for (i = 1; i < NLOOPS + 1; i += 2) {
			WAIT_PARENT_SIGNAL();
			if ((counter = update((long*)area)) != i)
				err_quit("parent: expected {}, taken {}", i, counter);
			TELL_PARENT_SIGNAL(getppid());
		}
	}
	exit(0);
}