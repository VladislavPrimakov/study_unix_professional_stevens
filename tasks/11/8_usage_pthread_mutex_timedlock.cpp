#include "apue.h"
#include <chrono>
#include <pthread.h>
using namespace std::chrono_literals;

struct timespec to_timespec(std::chrono::system_clock::time_point& tp) {
	auto secs = time_point_cast<std::chrono::seconds>(tp);
	auto ns = duration_cast<std::chrono::nanoseconds>(tp - secs);
	struct timespec ts;
	ts.tv_sec = secs.time_since_epoch().count();
	ts.tv_nsec = ns.count();
	return ts;
}

int main(void) {
	int err;
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&lock);
	std::println("mutex locked");
	auto utc_time = std::chrono::system_clock::now();
	auto local_time = std::chrono::zoned_time(std::chrono::current_zone(), utc_time);
	std::println("current time: {:%r}", local_time);
	utc_time += 5s;
	auto ts = to_timespec(utc_time);
	err = pthread_mutex_timedlock(&lock, &ts);
	utc_time = std::chrono::system_clock::now();
	local_time = std::chrono::zoned_time(std::chrono::current_zone(), utc_time);
	std::println("current time: {:%r}", local_time);
	if (err == 0)
		std::println("mutex locked again!");
	else
		std::println("didn't success to lock mutex again: {}", strerror(err));
	exit(0);
}