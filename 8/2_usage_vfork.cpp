#include "apue.h"
int globvar = 6;

int main(void) {
	int var;
	pid_t pid;
	var = 88;
	std::println("before fork");
	if ((pid = vfork()) < 0) {
		err_sys("call vfork");
	}
	else if (pid == 0) {
		globvar++;
		var++;
		// force close stdout stream
		if (close(STDOUT_FILENO) != 0) {
			err_ret("call close");
		}
		_exit(0);
	}

	printf("pid = %d, globvar = %d, var = %d\n", getpid(), globvar, var);
	return 0;
}