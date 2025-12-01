#include "apue.h"

void sig_quit(int);

int main() {
	sigset_t newmask, oldmask, pendmask;
	if (signal(SIGQUIT, sig_quit) == SIG_ERR) {
		err_sys("call signal(SIGQUIT)");
	}
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGQUIT);
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
		err_sys("call sigprocmask(SIG_BLOCK)");
	sleep(5);
	if (sigpending(&pendmask) < 0) {
		err_sys("call sigpending");
	}
	if (sigismember(&pendmask, SIGQUIT)) {
		std::println("\nsignal SIGQUIT is waiting for handle");
	}
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
		err_sys("call sigprocmask(SIG_SETMASK)");
	}
	std::println("signal SIGQUIT unblocked");
	sleep(5);
	return 0;
}

void sig_quit(int signo) {
	std::println("handling signal SIGQUIT");
	if (signal(SIGQUIT, SIG_DFL) == SIG_ERR)
		err_sys("cannot reset disposition for signal SIGQUIT");
}