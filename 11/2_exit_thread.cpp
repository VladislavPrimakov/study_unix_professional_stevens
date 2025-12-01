#include "apue.h"
#include <pthread.h>

void* thr_fn1(void* arg) {
	std::println("thread 1: exit");
	return((void*)1);
}

void* thr_fn2(void* arg) {
	std::println("thread 2: exit");
	pthread_exit((void*)2);
}

int main() {
	int err;
	pthread_t tid1, tid2;
	void* tret;
	err = pthread_create(&tid1, NULL, thr_fn1, NULL);
	if (err != 0)
		err_exit(err, "pthread_create 1");
	err = pthread_create(&tid2, NULL, thr_fn2, NULL);
	if (err != 0)
		err_exit(err, "pthread_create 2");
	err = pthread_join(tid1, &tret);
	if (err != 0)
		err_exit(err, "pthread_join 1");
	std::println("thread 1 exit code: {}", tret);
	err = pthread_join(tid2, &tret);
	if (err != 0)
		err_exit(err, "pthread_join 2");
	std::println("thread 2 exit code: {}", tret);
	exit(0);
}