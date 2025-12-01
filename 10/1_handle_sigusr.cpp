#include "apue.h"

static void sig_usr(int);

int main(void) {
	if (signal(SIGUSR1, sig_usr) == SIG_ERR)
		err_sys("handle signal SIGUSR1");
	if (signal(SIGUSR2, sig_usr) == SIG_ERR)
		err_sys("handle signal SIGUSR2");
	for (; ; )
		pause();
}

void sig_usr(int signo) {
	if (signo == SIGUSR1)
		std::println("taken signal SIGUSR1");
	else if (signo == SIGUSR2)
		std::println("taken signal SIGUSR2");
	else
		err_dump("taken signal {}", signo);
}