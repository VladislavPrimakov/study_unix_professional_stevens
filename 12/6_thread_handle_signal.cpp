#include "apue.h"
#include <pthread.h>

bool quitflag = false;
sigset_t mask;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t waitloc = PTHREAD_COND_INITIALIZER;

void* thr_fn(void* arg) {
	int err, signo;
	while (true) {
		err = sigwait(&mask, &signo);
		if (err != 0)
			err_exit(err, "call sigwait");
		switch (signo) {
		case SIGINT:
			std::println("\nSIGINT");
			break;
		case SIGQUIT:
			std::println("\nSIGQUIT");
			pthread_mutex_lock(&lock);
			quitflag = true;
			pthread_mutex_unlock(&lock);
			pthread_cond_signal(&waitloc);
			return 0;
		default:
			std::println("\ntaken unforseen signal {}", signo);
			exit(1);
		}
	}
}

int main() {
	int err;
	sigset_t oldmask;
	pthread_t tid;
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);
	if ((err = pthread_sigmask(SIG_BLOCK, &mask, &oldmask)) != 0)
		err_exit(err, "call pthread_sigmask(SIG_BLOCK)");
	err = pthread_create(&tid, NULL, thr_fn, 0);
	if (err != 0)
		err_exit(err, "call pthread_create");
	pthread_mutex_lock(&lock);
	while (!quitflag)
		pthread_cond_wait(&waitloc, &lock);
	pthread_mutex_unlock(&lock);
	quitflag = 0;
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		err_sys("call sigprocmask(SIG_SETMASK)");
	exit(0);
}