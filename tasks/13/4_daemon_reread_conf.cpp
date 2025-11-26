#include <apue.h>

void reread(void) {
	/* ... */
}

void sigterm(int signo) {
	syslog(LOG_INFO, "taken signal SIGTERM");
	exit(0);
}

void sighup(int signo) {
	syslog(LOG_INFO, "reading conf file");
	reread();
}

int main(int argc, char* argv[]) {
	char* cmd;
	struct sigaction sa;
	// remove chars before slash
	if ((cmd = strrchr(argv[0], '/')) == NULL)
		cmd = argv[0];
	else
		cmd++;
	daemonize(cmd);
	if (already_running()) {
		syslog(LOG_ERR, "daemon has already run");
		exit(1);
	}
	sa.sa_handler = sigterm;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGHUP);
	sa.sa_flags = 0;
	if (sigaction(SIGTERM, &sa, NULL) < 0) {
		syslog(LOG_ERR, "error sigaction(SIGTERM): %s", strerror(errno));
		exit(1);
	}
	sa.sa_handler = sighup;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGTERM);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) < 0) {
		syslog(LOG_ERR, "error sigaction(SIGHUP): %s", strerror(errno));
		exit(1);
	}
	while (true) {
		pause();
	}
	exit(0);
}