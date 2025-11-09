#include "apue.h"

int main(void) {
	pid_t pid;
	int status;
	if ((pid = fork()) < 0)
		err_sys("call fork");
	else if (pid == 0)
		exit(7);
	if (wait(&status) != pid)
		err_sys("call wait");
	pr_exit(status);
	if ((pid = fork()) < 0)
		err_sys("call fork");
	else if (pid == 0)
		abort();
	if (wait(&status) != pid)
		err_sys("call wait");
	pr_exit(status);
	if ((pid = fork()) < 0)
		err_sys("call fork");
	else if (pid == 0)
		status /= 0; /* dividing by zero genereta signal SIGFPE */
	if (wait(&status) != pid)
		err_sys("call wait");
	pr_exit(status);
	return 0;
}