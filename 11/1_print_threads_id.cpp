#include "apue.h"
#include <pthread.h>

pthread_t ntid;

void printids(const std::string& s) {
	std::println("{} pid {} tid {}", s, getpid(), gettid());
}

void* thr_fn(void* arg) {
	printids("new thread: ");
	return((void*)0);
}

int main() {
	int err;
	err = pthread_create(&ntid, NULL, thr_fn, NULL);
	if (err != 0)
		err_exit(err, "pthread_create");
	printids("main thread: ");
	sleep(1);
	exit(0);
}