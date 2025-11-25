#include "apue.h"
#include <pthread.h>

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

void prepare() {
	int err;
	std::println("preparing blocks...");
	if ((err = pthread_mutex_lock(&lock1)) != 0)
		err_cont(err, "call pthread_mutex_lock(lock1) at prepare");
	if ((err = pthread_mutex_lock(&lock2)) != 0)
		err_cont(err, "call pthread_mutex_lock(lock2) at prepare");
}

void parent() {
	int err;
	std::println("parent is unlocking blocks...");
	if ((err = pthread_mutex_unlock(&lock1)) != 0)
		err_cont(err, "call pthread_mutex_unlock(lock1) at parent");
	if ((err = pthread_mutex_unlock(&lock2)) != 0)
		err_cont(err, "call pthread_mutex_unlock(lock2) at parent");
}

void child() {
	int err;
	std::println("child is unlocking blocks...");
	if ((err = pthread_mutex_unlock(&lock1)) != 0)
		err_cont(err, "call pthread_mutex_unlock(lock1) at child");
	if ((err = pthread_mutex_unlock(&lock2)) != 0)
		err_cont(err, "call pthread_mutex_unlock(lock2) at child");
}

void* thr_fn(void* arg) {
	std::println("thread started...");
	pause();
	return(0);
}

int main() {
	int err;
	pid_t pid;
	pthread_t tid;
	if ((err = pthread_atfork(prepare, parent, child)) != 0)
		err_exit(err, "call pthread_atfork");
	if ((err = pthread_create(&tid, NULL, thr_fn, 0)) != 0)
		err_exit(err, "call pthread_create");
	sleep(2);
	std::println("parent process is calling fork...");
	if ((pid = fork()) < 0)
		err_quit("call fork");
	else if (pid == 0)
		std::println("child process {} after fork", getpid());
	else
		std::println("parent process {} after fork", getpid());
	exit(0);
}