#include "apue.h"

struct timespec to_timespec(const std::chrono::system_clock::time_point& tp) {
	using namespace std::chrono;
	auto duration = tp.time_since_epoch();
	auto sec = duration_cast<seconds>(duration);
	auto nanosec = duration_cast<nanoseconds>(duration - sec);
	return {
		static_cast<time_t>(sec.count()),
		static_cast<long>(nanosec.count())
	};
}

std::chrono::system_clock::time_point to_time_point(const struct timespec& ts) {
	using namespace std::chrono;
	auto duration = seconds{ ts.tv_sec } + nanoseconds{ ts.tv_nsec };
	return system_clock::time_point(duration);
}