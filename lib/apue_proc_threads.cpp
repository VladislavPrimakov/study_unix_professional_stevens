#include "apue.h"

Sigfunc* apue_signal(int signo, Sigfunc* func) {
	struct sigaction act, oact;
	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;
#endif
	}
	else {
		act.sa_flags |= SA_RESTART;
	}
	if (sigaction(signo, &act, &oact) < 0) {
		return(SIG_ERR);
	}
	return(oact.sa_handler);
}

int makethread(ThreadFunc fn, void* arg) {
	int err;
	pthread_t tid;
	pthread_attr_t attr;
	err = pthread_attr_init(&attr);
	if (err != 0)
		return(err);
	err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (err == 0)
		err = pthread_create(&tid, &attr, fn, arg);
	pthread_attr_destroy(&attr);
	return(err);
}

int system(const char* cmdstring) {
	pid_t pid;
	int status;
	if (cmdstring == NULL)
		return(1);
	if ((pid = fork()) < 0) {
		status = -1;
	}
	else if (pid == 0) {
		execl("/bin/sh", "sh", "-c", cmdstring, (char*)0);
		_exit(127);
	}
	else {
		while (waitpid(pid, &status, 0) < 0) {
			if (errno != EINTR) {
				status = -1;
				break;
			}
		}
	}
	return(status);
}

void pr_mask(const std::string& str) {
	sigset_t sigset;
	int errno_save;
	errno_save = errno;
	std::string s = str;
	if (sigprocmask(0, NULL, &sigset) < 0) {
		err_ret("call sigprocmask");
	}
	else {
		bool first = false;
		for (int i = 1; i < NSIG; ++i) {
			if (sigismember(&sigset, i)) {
				if (!first) {
					first = true;
					s += " " + std::string(strsignal(i));
				}
				else {
					s += " | " + std::string(strsignal(i));
				}
			}
		}
		std::println("\n{}", s);
	}
	errno = errno_save;
}

void pr_exit(int status) {
	if (WIFEXITED(status))
		std::println("Normal exit, exit code = {}", WEXITSTATUS(status));
	else if (WIFSIGNALED(status))
		std::println("Abort, exit code = {}{}", WTERMSIG(status),
#ifdef WCOREDUMP
			WCOREDUMP(status) ? " (created dump core)" : "");
#else
		"");
#endif
	else if (WIFSTOPPED(status))
		std::println("Child process stopped, code signal = {}", WSTOPSIG(status));
}