#include <signal.h>
#include <unistd.h>

void sig_alrm(int signo) {
	// do nothing
}

unsigned int sleep1(unsigned int seconds) {
	if (signal(SIGALRM, sig_alrm) == SIG_ERR)
		return(seconds);
	alarm(seconds);
	pause();
	return(alarm(0));
}

int main() {
	sleep1(5);
	return 0;
}