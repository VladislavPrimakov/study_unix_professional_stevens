#include "apue.h"

int main() {
	pid_t pid;
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid == 0) { /* first child */
		if ((pid = fork()) < 0) {
			err_sys("call fork");
		}
		else if (pid > 0) {
			exit(0); /* first child */
		}
		// second child with parent as init process
		sleep(2);
		std::println("second child, ppid = {}", getppid());
		exit(0);
	}
	if (waitpid(pid, NULL, 0) != pid)
		err_sys("call waitpid");
	exit(0);
}