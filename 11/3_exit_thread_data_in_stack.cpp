#include "apue.h"
#include <pthread.h>

struct foo {
	int a, b, c, d;
	void print(const std::string& s) {
		std::println("{} adress: {}", s, static_cast<const void*>(this));
		std::println(" foo.a = {}", a);
		std::println(" foo.b = {}", b);
		std::println(" foo.c = {}", c);
		std::println(" foo.d = {}", d);
	}
};


void* thr_fn1(void* arg) {
	struct foo foo = { 1, 2, 3, 4 };
	foo.print("thread 1:");
	pthread_exit((void*)&foo);
}

void* thr_fn2(void* arg) {
	std::println("thread 2: tid - {}", gettid());
	pthread_exit((void*)0);
}

int main() {
	int err;
	pthread_t tid1, tid2;
	struct foo* fp;
	void* thread_return_value;
	err = pthread_create(&tid1, NULL, thr_fn1, NULL);
	if (err != 0)
		err_exit(err, "pthread_create 1");
	err = pthread_join(tid1, &thread_return_value);
	if (err != 0)
		err_exit(err, "pthread_join 1");
	fp = static_cast<struct foo*>(thread_return_value);
	sleep(1);
	std::println("main thread creates child thread");
	err = pthread_create(&tid2, NULL, thr_fn2, NULL);
	if (err != 0)
		err_exit(err, "pthread_create 2");
	sleep(1);
	fp->print("main thread:");
	exit(0);
}