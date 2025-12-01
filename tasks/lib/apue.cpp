#include "apue.h"

ssize_t readn(int fd, void* ptr, size_t n) {
	size_t nleft;
	ssize_t nread;
	nleft = n;
	while (nleft > 0) {
		if ((nread = read(fd, ptr, nleft)) < 0) {
			if (nleft == n)
				return(-1);
			else
				break;
		}
		else if (nread == 0) {
			break;
		}
		nleft -= nread;
		ptr += nread;
	}
	return(n - nleft);
}

ssize_t writen(int fd, const void* ptr, size_t n) {
	size_t nleft;
	ssize_t nwritten;
	nleft = n;
	while (nleft > 0) {
		if ((nwritten = write(fd, ptr, nleft)) < 0) {
			if (nleft == n)
				return(-1);
			else
				break;
		}
		else if (nwritten == 0) {
			break;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return(n - nleft);
}

struct timespec to_timespec(const std::chrono::system_clock::time_point& tp) {
	using namespace std::chrono;
	auto duration = tp.time_since_epoch();
	auto sec = duration_cast<seconds>(duration);
	auto nanosec = duration_cast<nanoseconds>(duration - sec);
	return {
		static_cast<time_t>(sec.count()),
		static_cast<long>(nanosec.count())
	};
}


std::chrono::system_clock::time_point to_time_point(const struct timespec& ts) {
	using namespace std::chrono;
	auto duration = seconds{ ts.tv_sec } + nanoseconds{ ts.tv_nsec };
	return system_clock::time_point(duration);
}

int set_cloexec(int fd) {
	int val;
	if ((val = fcntl(fd, F_GETFD, 0)) < 0)
		return(-1);
	val |= FD_CLOEXEC;
	return(fcntl(fd, F_SETFD, val));
}

int lockfile(int fd) {
	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	return(fcntl(fd, F_SETLK, &fl));
}

void daemonize(const char* cmd) {
	int i, fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;
	umask(0);

	if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
		err_quit("{}: cant get max descriptor number", cmd);

	if ((pid = fork()) < 0)
		err_quit("{}: call fork", cmd);
	else if (pid != 0)
		exit(0);
	setsid();

	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) < 0)
		err_quit("{}: call sigaction to ignore SIGHUP", cmd);
	if ((pid = fork()) < 0)
		err_quit("{}: call fork", cmd);
	else if (pid != 0)
		exit(0);

	if (chdir("/") < 0)
		err_quit("{}: call chdir(\"/\")", cmd);

	if (rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for (i = 0; i < rl.rlim_max; i++)
		close(i);

	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	openlog(cmd, LOG_CONS, LOG_DAEMON);
	if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
		syslog(LOG_ERR, "error fd %d %d %d", fd0, fd1, fd2);
		exit(1);
	}
}

int already_running(void) {
	int fd;
	fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);
	if (fd < 0) {
		syslog(LOG_ERR, "can't open %s: %s", LOCKFILE, strerror(errno));
		exit(1);
	}
	if (lockfile(fd) < 0) {
		if (errno == EACCES || errno == EAGAIN) {
			close(fd);
			return(1);
		}
		syslog(LOG_ERR, "can't set block for %s: %s", LOCKFILE, strerror(errno));
		exit(1);
	}
	if (ftruncate(fd, 0) < 0) {
		err_sys("ftruncate error");
	}
	std::string str_buf = std::to_string(getpid());
	if (write(fd, str_buf.data(), str_buf.size()) < str_buf.size()) {
		err_quit("call write");
	}
	return(0);
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

int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len) {
	struct flock lock;
	lock.l_type = type; /* F_RDLCK, F_WRLCK, F_UNLCK */
	lock.l_start = offset;
	lock.l_whence = whence; /* SEEK_SET, SEEK_CUR, SEEK_END */
	lock.l_len = len;
	return(fcntl(fd, cmd, &lock));
}

pid_t lock_test(int fd, int type, off_t offset, int whence, off_t len) {
	struct flock lock;
	lock.l_type = type;
	lock.l_start = offset;
	lock.l_whence = whence;
	lock.l_len = len;
	if (fcntl(fd, F_GETLK, &lock) < 0)
		err_sys("fcntl error");
	if (lock.l_type == F_UNLCK)
		return(0);
	return(lock.l_pid);
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
Sigfunc* old_handler_usr1, * old_handler_usr2;

void sig_usr(int signo) {
	sigflag = 1;
}

void TELL_WAIT() {
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

void TELL_DONE() {
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
		err_sys("call sigprocmask(SIG_SETMASK)");
	}
	if (apue_signal(SIGUSR1, old_handler_usr1) == SIG_ERR)
		err_sys("call signal(SIGUSR1)");
	if (apue_signal(SIGUSR2, old_handler_usr2) == SIG_ERR)
		err_sys("call signal(SIGUSR2)");
}

void TELL_PARENT(pid_t pid) {
	kill(pid, SIGUSR2); /* tell parent we are ready */
}

void WAIT_PARENT() {
	while (sigflag == 0)
		sigsuspend(&zeromask); /* wait for answer from parent */
	sigflag = 0;
}

void TELL_CHILD(pid_t pid) {
	kill(pid, SIGUSR1); /* tell child we are ready */
}

void WAIT_CHILD() {
	while (sigflag == 0)
		sigsuspend(&zeromask); /* wait for answer from child */
	sigflag = 0;
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