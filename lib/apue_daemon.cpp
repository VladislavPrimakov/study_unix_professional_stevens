#include "apue.h"

void daemonize(const char* cmd) {
	int fd0, fd1, fd2;
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
	for (rlim_t i = 0; i < rl.rlim_max; i++)
		close(i);

	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	openlog(cmd, LOG_CONS, LOG_DAEMON);
	if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
		syslog(LOG_ERR, "error fd %d %d %d", fd0, fd1, fd2);
		exit(1);
	}

	log_to_stderr = false;
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
	if (!writen(fd, str_buf.data(), str_buf.size())) {
		err_quit("call write");
	}
	return(0);
}