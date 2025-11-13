#include <apue.h>

static void sig_cld(int);

int main() {
	pid_t pid;
	if (signal(SIGCLD, sig_cld) == SIG_ERR)
		err_ret("call signal SIGCLD");
	if ((pid = fork()) < 0) {
		err_ret("call fork");
	}
	else if (pid == 0) {
		sleep(2);
		_exit(0);
	}
	pause();
	exit(0);
}

void sig_cld(int signo) {
	pid_t pid;
	int status;
	std::println("taken signal SIGCLD");
	if (signal(SIGCLD, sig_cld) == SIG_ERR)
		err_ret("call signal SIGCLD");
	if ((pid = wait(&status)) < 0) {
		err_ret("call wait");
	}
	std::println("parentpid = {}, childpid = {}", getpid(), pid);
}