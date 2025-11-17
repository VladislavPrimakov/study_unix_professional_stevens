//Используя единственный системный таймер(alarm или setitimer — таймер с высоким разрешением), 
// реализуйте набор функций, которые предоставляли бы в распоряжение процесса произвольное количество таймеров.

#include <algorithm>
#include <apue.h>
#include <set>

std::set<std::pair<int, Sigfunc*>> alarm_set;
bool set_handler = false;

void apue_reset_alarm(int seconds, Sigfunc* f = nullptr) {
	int t = alarm(0);
	std::set<std::pair<int, Sigfunc*>> temp_storage;
	for (const auto& t : alarm_set) {
		int new_time = t.first - seconds;
		if (new_time > 0) {
			temp_storage.insert({ seconds, f });
		}
	}
	alarm_set = std::move(temp_storage);
	if (f != nullptr) {
		alarm_set.insert({ seconds, f });
	}
	sigset_t zeromask;
	sigemptyset(&zeromask);
	alarm(alarm_set.begin()->first);
	sigsuspend(&zeromask);
}

void apue_alarm_handler(int signo) {
	if (!alarm_set.empty()) {
		auto func = alarm_set.begin()->second;
		func(signo);
		apue_reset_alarm(alarm_set.begin()->first);
	}
}


void apue_alarm(int seconds, Sigfunc* f) {
	if (seconds <= 0) return;

	sigset_t newmask, oldmask;
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGALRM);
	sigprocmask(SIG_BLOCK, &newmask, &oldmask);

	if (!set_handler) {
		struct sigaction act;
		act.sa_handler = apue_alarm_handler;
		sigemptyset(&act.sa_mask);
		act.sa_flags = 0;
		act.sa_flags |= SA_RESTART;
		if (sigaction(SIGALRM, &act, NULL) < 0) {
			err_sys("sigaction");
		}
		set_handler = true;
	}

	apue_reset_alarm(seconds, f);
	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

void custom_alarm_handler(int signo) {
	std::println("Alarm signal received: {}", signo);
}

int main() {
	apue_alarm(3, custom_alarm_handler);
	apue_alarm(5, custom_alarm_handler);
	apue_alarm(2, custom_alarm_handler);
	return 0;
}