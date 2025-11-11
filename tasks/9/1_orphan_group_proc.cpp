#include "apue.h"

void sig_hup(int signo) {
	std::println("taken signal SIGHUP, pid = {}", getpid());
}

void pr_ids(std::string name) {
	std::println("{}: pid = {}, ppid = {}, pgrp = {}, tpgrp = {}", name, getpid(), getppid(), getpgrp(), tcgetpgrp(STDIN_FILENO));
	fflush(stdout);
}

int main() {
	char c;
	pid_t pid;
	pr_ids("parent");
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid > 0) { // parent
		sleep(5);
	}
	else { // child
		pr_ids("child before SIGTSTP");
		signal(SIGHUP, sig_hup);
		kill(getpid(), SIGTSTP);
		pr_ids("child after SIGTSTP");
		if (read(STDIN_FILENO, &c, 1) != 1)
			err_ret("read from control TTY");
	}
	exit(0);
}