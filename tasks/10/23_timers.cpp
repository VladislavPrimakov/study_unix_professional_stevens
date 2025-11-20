//Используя единственный системный таймер(alarm или setitimer — таймер с высоким разрешением), 
// реализуйте набор функций, которые предоставляли бы в распоряжение процесса произвольное количество таймеров.

#include <algorithm>
#include "apue.h"
#include <chrono>
#include <set>

static std::set<std::pair<int, Sigfunc*>> alarm_set;
static bool handler_is_set = false;

void apue_alarm_handler(int signo);

void reset_system_alarm() {
	if (alarm_set.empty()) {
		alarm(0);
	}
	else {
		alarm(alarm_set.begin()->first);
	}
}

void apue_alarm(int seconds, Sigfunc* func) {
	if (seconds <= 0 || func == nullptr) return;

	sigset_t newmask, oldmask;
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGALRM);
	sigprocmask(SIG_BLOCK, &newmask, &oldmask);

	if (!handler_is_set) {
		struct sigaction act;
		act.sa_handler = apue_alarm_handler;
		sigemptyset(&act.sa_mask);
		act.sa_flags = SA_RESTART;
		if (sigaction(SIGALRM, &act, NULL) < 0) {
			err_sys("sigaction(SIGALRM) failed");
		}
		handler_is_set = true;
	}

	int elapsed_time = alarm(0);

	std::set<std::pair<int, Sigfunc*>> temp_set;
	if (elapsed_time > 0) {
		for (const auto& timer_pair : alarm_set) {
			temp_set.insert({ timer_pair.first - (alarm_set.begin()->first - elapsed_time), timer_pair.second });
		}
		alarm_set = std::move(temp_set);
	}

	alarm_set.insert({ seconds, func });
	reset_system_alarm();
	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

void apue_alarm_handler(int signo) {
	if (alarm_set.empty()) return;
	int elapsed_time = alarm_set.begin()->first;
	Sigfunc* handler_to_call = alarm_set.begin()->second;
	handler_to_call(signo);
	alarm_set.erase(alarm_set.begin());
	std::set<std::pair<int, Sigfunc*>> temp_set;
	for (const auto& timer_pair : alarm_set) {
		temp_set.insert({ timer_pair.first - elapsed_time, timer_pair.second });
	}
	alarm_set = std::move(temp_set);
	reset_system_alarm();
}

void wait_apue_timers() {
	sigset_t newmask, oldmask, zeromask;
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGALRM);
	sigemptyset(&zeromask);
	sigprocmask(SIG_BLOCK, &newmask, &oldmask);
	while (!alarm_set.empty()) {
		sigsuspend(&zeromask);
	}
	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

std::chrono::steady_clock::time_point start;

void custom_alarm_handler(int signo) {
	std::println("Time elapsed: {}", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start));
}


int main() {
	start = std::chrono::steady_clock::now();
	apue_alarm(3, custom_alarm_handler);
	apue_alarm(5, custom_alarm_handler);
	apue_alarm(2, custom_alarm_handler);
	wait_apue_timers();
	return 0;
}