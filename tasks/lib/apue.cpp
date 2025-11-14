#include <apue.h>

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


void clr_fl(int fd, int flags) {
	int val;
	if ((val = fcntl(fd, F_GETFL, 0)) < 0)
		err_ret("call fcntl with F_GETFL");
	val &= ~flags;
	if (fcntl(fd, F_SETFL, val) < 0)
		err_ret("call fcntl with F_SETFL");
}


void set_fl(int fd, int flags) {
	int val;
	if ((val = fcntl(fd, F_GETFL, 0)) < 0)
		err_ret("call fcntl with F_GETFL");
	val |= flags;
	if (fcntl(fd, F_SETFL, val) < 0)
		err_ret("call fcntl with F_SETFL");
}


std::string path_alloc() {
	static std::size_t pathmax = 0;
	static std::size_t posix_version = 0;
	static std::size_t xsi_version = 0;

	if (posix_version == 0)
		posix_version = sysconf(_SC_VERSION);
	if (xsi_version == 0)
		xsi_version = sysconf(_SC_XOPEN_VERSION);

	if (pathmax == 0) {
#ifdef PATH_MAX
		pathmax = PATH_MAX;
#else
		pathmax = 0;
#endif
		if (pathmax == 0) {
			errno = 0;
			if ((pathmax = pathconf("/", _PC_PATH_MAX)) < 0) {
				if (errno == 0)
					pathmax = PATH_MAX_GUESS;
				else
					err_ret("pathconf error for _PC_PATH_MAX");
			}
			else {
				pathmax++;
			}
		}
	}

	try {
		std::string buffer;
		buffer.resize_and_overwrite(pathmax, [](char* buf, std::size_t count) { return count; });
		return buffer;
	}
	catch (const std::bad_alloc&) {
		throw std::runtime_error("malloc error for path_alloc");
	}
}


long open_max() {
	static long openmax = 0;
#ifdef OPEN_MAX
	openmax = OPEN_MAX;
#endif
	if (openmax == 0) {
		errno = 0;
		if ((openmax = sysconf(_SC_OPEN_MAX)) < 0) {
			if (errno == 0) {
				openmax = OPEN_MAX_GUESS;
			}
			else {
				err_ret("sysconf error for _SC_OPEN_MAX");
			}
		}
	}
	return openmax;
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


volatile sig_atomic_t sigflag;
sigset_t newmask, oldmask, zeromask;

void sig_usr(int signo) {
	sigflag = 1;
}

void TELL_WAIT(void) {
	if (signal(SIGUSR1, sig_usr) == SIG_ERR)
		err_sys("call signal(SIGUSR1)");
	if (signal(SIGUSR2, sig_usr) == SIG_ERR)
		err_sys("call signal(SIGUSR2)");
	sigemptyset(&zeromask);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGUSR1);
	sigaddset(&newmask, SIGUSR2);

	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
		err_sys("call SIG_BLOCK");
}

void TELL_PARENT(pid_t pid) {
	kill(pid, SIGUSR2); /* tell parent we are ready */
}

void WAIT_PARENT(void) {
	while (sigflag == 0)
		sigsuspend(&zeromask); /* wait for answer from parent */
	sigflag = 0;
	// reset mask
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		err_sys("call SIG_SETMASK");
}

void TELL_CHILD(pid_t pid) {
	kill(pid, SIGUSR1); /* tell child we are ready */
}

void WAIT_CHILD(void) {
	while (sigflag == 0)
		sigsuspend(&zeromask); /* wawit for answer from child */
	sigflag = 0;
	// reset mask
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		err_sys("call SIG_SETMASK");
}


void pr_mask(const std::string& str) {
	sigset_t sigset;
	int errno_save;
	errno_save = errno;
	if (sigprocmask(0, NULL, &sigset) < 0) {
		err_ret("call sigprocmask");
	}
	else {
		std::print("{}", str);
		if (sigismember(&sigset, SIGINT))
			std::print(" SIGINT ");
		if (sigismember(&sigset, SIGQUIT))
			std::print(" SIGQUIT ");
		if (sigismember(&sigset, SIGUSR1))
			std::print(" SIGUSR1 ");
		if (sigismember(&sigset, SIGALRM))
			std::print(" SIGALRM ");
		// continue for all signals you want to check
		std::println();
	}
	errno = errno_save;
}