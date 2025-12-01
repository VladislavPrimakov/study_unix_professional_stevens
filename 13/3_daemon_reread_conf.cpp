#include "apue.h"
#include <pthread.h>

sigset_t mask;

void reread(void) {
	/* ... */
}

void* thr_fn(void* arg) {
	int err, signo;
	while (true) {
		err = sigwait(&mask, &signo);
		if (err != 0) {
			syslog(LOG_ERR, "call sigwait");
			exit(1);
		}
		switch (signo) {
		case SIGHUP:
			syslog(LOG_INFO, "reading conf file");
			reread();
			break;
		case SIGTERM:
			syslog(LOG_INFO, "taken SIGTERM");
			exit(0);
		default:
			syslog(LOG_INFO, "taken unforseen signal %d", signo);
		}
	}
	return(0);
}

int main(int argc, char* argv[]) {
	int err;
	pthread_t tid;
	char* cmd;
	struct sigaction sa;
	if ((cmd = strrchr(argv[0], '/')) == NULL)
		cmd = argv[0];
	else
		cmd++;
	daemonize(cmd);
	if (already_running()) {
		syslog(LOG_ERR, "daemon already running");
		exit(1);
	}
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) < 0)
		err_quit("call sigaction(SIG_DFL for SIGHUP)");
	sigfillset(&mask);
	if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0)
		err_exit(err, "pthread_sigmask(SIG_BLOCK)");
	err = pthread_create(&tid, NULL, thr_fn, 0);
	if (err != 0)
		err_exit(err, "pthread_create");
	pause();
	exit(0);
}