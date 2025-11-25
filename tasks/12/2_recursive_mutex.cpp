#include "apue.h"
#include <chrono>
#include <pthread.h>

struct to_info {
	ThreadFunc to_fn;
	void* to_arg;
	struct timespec absolute_time;
};

// thread job
void* timeout_helper(void* arg) {
	struct to_info* tip = (struct to_info*)arg;
	std::println("Current time: {:%T}", std::chrono::system_clock::now());
	std::println("Wake up scheduled at: {:%T}", to_time_point(tip->absolute_time));
	clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &tip->absolute_time, NULL);
	(*tip->to_fn)(tip->to_arg);
	delete tip;
	return(0);
}

// run thread to call func at time point when
void timeout(const std::chrono::system_clock::time_point& when, ThreadFunc func, void* arg) {
	struct to_info* tip = new to_info();
	tip->to_fn = func;
	tip->to_arg = arg;
	tip->absolute_time = to_timespec(when);
	int err = makethread(timeout_helper, (void*)tip);
	if (err == 0)
		return;
	else {
		delete tip;
		// call if can't create thread
		(*func)(arg);
	}
}

pthread_mutexattr_t attr;
pthread_mutex_t mutex;

void* retry(void* arg) {
	pthread_mutex_lock(&mutex);
	std::println("Retry, arg = {}", arg);
	pthread_mutex_unlock(&mutex);
	return 0;
}

int main() {
	using namespace std::chrono_literals;
	int err = 0, condition = 1, arg = 1;
	if ((err = pthread_mutexattr_init(&attr)) != 0)
		err_exit(err, "call pthread_mutexattr_init");
	if ((err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) != 0)
		err_exit(err, "call pthread_mutexattr_settype(PTHREAD_MUTEX_RECURSIVE)");
	if ((err = pthread_mutex_init(&mutex, &attr)) != 0)
		err_exit(err, "pthread_mutex_init");

	pthread_mutex_lock(&mutex);
	if (condition) {
		auto when = std::chrono::system_clock::now() + 2s;
		timeout(when, retry, (void*)(unsigned long)(arg));
	}
	pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);
	exit(0);
}