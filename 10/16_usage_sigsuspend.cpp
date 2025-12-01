#include "apue.h"

volatile sig_atomic_t quitflag;

void sig_int(int signo) {
	if (signo == SIGINT)
		std::println("\ninterrupt");
	else if (signo == SIGQUIT)
		quitflag = 1;
}

int main() {
	sigset_t newmask, oldmask, zeromask;
	if (signal(SIGINT, sig_int) == SIG_ERR)
		err_sys("call signal(SIGINT)");
	if (signal(SIGQUIT, sig_int) == SIG_ERR)
		err_sys("call signal(SIGQUIT)");
	sigemptyset(&zeromask);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGQUIT);
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
		err_sys("call sigprocmask(SIG_BLOCK)");
	while (quitflag == 0)
		sigsuspend(&zeromask);
	quitflag = 0;
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		err_sys("call sigprocmask(SIG_SETMASK)");
	return 0;
}