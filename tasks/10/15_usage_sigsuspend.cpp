#include "apue.h"

void sig_int(int);

int main() {
	sigset_t newmask, oldmask, waitmask;
	pr_mask("in the beginning: ");
	if (apue_signal(SIGINT, sig_int) == SIG_ERR)
		err_sys("call signal(SIGINT)");
	sigemptyset(&waitmask);
	sigaddset(&waitmask, SIGUSR1);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGINT);
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
		err_sys("call sigprocmask(SIG_BLOCK)");
	pr_mask("inside critical code: ");
	if (sigsuspend(&waitmask) != -1)
		err_sys("call sigsuspend");
	pr_mask("after sigsuspend: ");
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		err_sys("call sigprocmask(SIG_SETMASK)");
	pr_mask("in the end: ");
	exit(0);
}

void sig_int(int signo) {
	pr_mask("in the sig_int: ");
}