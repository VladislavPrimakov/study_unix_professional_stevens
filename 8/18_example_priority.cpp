#include "apue.h"
#include <chrono>

using namespace std::chrono_literals;

int main(int argc, char* argv[]) {
	pid_t pid;
	std::string s;
	int nzero, ret;
	int adj = 0;
	setbuf(stdout, NULL);
#if defined(NZERO)
	nzero = NZERO;
#elif defined(_SC_NZERO)
	nzero = sysconf(_SC_NZERO);
#else
#error NZERO undefined
#endif
	std::println("NZERO = {}", nzero);
	if (argc == 2)
		adj = strtol(argv[1], NULL, 10);
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid == 0) {
		s = "child";
		std::println("nice in child {}, want to change to {}", nice(0) + nzero, adj + nzero);
		errno = 0;
		if ((ret = nice(adj)) == -1 && errno != 0)
			err_sys("call nice");
		std::println("nice in child {}", ret + nzero);
	}
	else {
		s = "parent";
		std::println("nice in parent {}", nice(0) + nzero);
	}

	auto start_time = std::chrono::steady_clock::now();
	unsigned long long count = 0;
	while (std::chrono::steady_clock::now() - start_time < 10s) {
		if (++count == 0)
			err_quit("{} counter overflow", s);
		count++;
	}
	std::println("{} counter = {}", s, count);
	exit(0);
}