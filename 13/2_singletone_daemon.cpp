#include <apue.h>


int main() {
	openlog("singletone_daemon", LOG_PID, LOG_DAEMON);
	if (already_running()) {
		syslog(LOG_ERR, "daemon already running");
		exit(1);
	}
	// Daemon code here
	while (1) {
		syslog(LOG_INFO, "daemon is running");
		pause();
	}
	return 0;
}