#include "apue.h"

#define BUFFSIZE 1024

void sig_tstp(int signo) {
	sigset_t mask;
	// some work before stop
	sigemptyset(&mask);
	sigaddset(&mask, SIGTSTP);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
	apue_signal(SIGTSTP, SIG_DFL);
	kill(getpid(), SIGTSTP);
	// until get SIGCONT
	apue_signal(SIGTSTP, sig_tstp);
}

int main() {
	int n;
	char buf[BUFFSIZE];
	if (apue_signal(SIGTSTP, SIG_IGN) == SIG_DFL)
		apue_signal(SIGTSTP, sig_tstp);
	while ((n = read(STDIN_FILENO, buf, BUFFSIZE)) > 0) {
		if (write(STDOUT_FILENO, buf, n) != n) {
			err_sys("call write");
		}
	}
	if (n < 0) {
		err_sys("call read");
	}
	return 0;
}