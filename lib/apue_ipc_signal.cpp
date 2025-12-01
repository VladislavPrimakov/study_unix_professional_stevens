#include "apue.h"

volatile sig_atomic_t sigflag;
sigset_t newmask, oldmask, zeromask;
Sigfunc* old_handler_usr1, * old_handler_usr2;

void sig_usr(int signo) {
	sigflag = 1;
}

void TELL_WAIT_SIGNAL() {
	if ((old_handler_usr1 = apue_signal(SIGUSR1, sig_usr)) == SIG_ERR)
		err_sys("call signal(SIGUSR1)");
	if ((old_handler_usr2 = apue_signal(SIGUSR2, sig_usr)) == SIG_ERR)
		err_sys("call signal(SIGUSR2)");
	sigemptyset(&zeromask);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGUSR1);
	sigaddset(&newmask, SIGUSR2);
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
		err_sys("call sigprocmask(SIG_BLOCK)");
}

void TELL_DONE_SIGNAL() {
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
		err_sys("call sigprocmask(SIG_SETMASK)");
	}
	if (apue_signal(SIGUSR1, old_handler_usr1) == SIG_ERR)
		err_sys("call signal(SIGUSR1)");
	if (apue_signal(SIGUSR2, old_handler_usr2) == SIG_ERR)
		err_sys("call signal(SIGUSR2)");
}

void TELL_PARENT_SIGNAL(pid_t pid) {
	kill(pid, SIGUSR2); /* tell parent we are ready */
}

void WAIT_PARENT_SIGNAL() {
	while (sigflag == 0)
		sigsuspend(&zeromask); /* wait for answer from parent */
	sigflag = 0;
}

void TELL_CHILD_SIGNAL(pid_t pid) {
	kill(pid, SIGUSR1); /* tell child we are ready */
}

void WAIT_CHILD_SIGNAL() {
	while (sigflag == 0)
		sigsuspend(&zeromask); /* wait for answer from child */
	sigflag = 0;
}