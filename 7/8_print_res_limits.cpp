#include "apue.h"
#include <string_view>
#include <sys/resource.h>

#define doit(name) pr_limits(#name, name)
void pr_limits(std::string_view, int);

int main(void) {
#ifdef RLIMIT_AS
	doit(RLIMIT_AS);
#endif
	doit(RLIMIT_CORE);
	doit(RLIMIT_CPU);
	doit(RLIMIT_DATA);
	doit(RLIMIT_FSIZE);
#ifdef RLIMIT_MEMLOCK
	doit(RLIMIT_MEMLOCK);
#endif
#ifdef RLIMIT_MSGQUEUE
	doit(RLIMIT_MSGQUEUE);
#endif
#ifdef RLIMIT_NICE
	doit(RLIMIT_NICE);
#endif
	doit(RLIMIT_NOFILE);
#ifdef RLIMIT_NPROC
	doit(RLIMIT_NPROC);
#endif
#ifdef RLIMIT_NPTS
	doit(RLIMIT_NPTS);
#endif
#ifdef RLIMIT_RSS
	doit(RLIMIT_RSS);
#endif
#ifdef RLIMIT_SBSIZE
	doit(RLIMIT_SBSIZE);
#endif
#ifdef RLIMIT_SIGPENDING
	doit(RLIMIT_SIGPENDING);
#endif
	doit(RLIMIT_STACK);
#ifdef RLIMIT_SWAP
	doit(RLIMIT_SWAP);
#endif
#ifdef RLIMIT_VMEM
	doit(RLIMIT_VMEM);
#endif
	return 0;
}

void pr_limits(std::string_view name, int resource) {
	struct rlimit limit;
	unsigned long long lim;
	if (getrlimit(resource, &limit) < 0)
		err_sys("call getrlimit for {}", name);
	std::print("{:<20} ", name);
	if (limit.rlim_cur == RLIM_INFINITY) {
		std::print("{:<10}", "(INF)");
	}
	else {
		lim = limit.rlim_cur;
		std::print("{:<10} ", lim);
	}
	if (limit.rlim_max == RLIM_INFINITY) {
		std::print("{:<10}", "(INF)");
	}
	else {
		lim = limit.rlim_max;
		std::print("{:<10} ", lim);
	}
	std::println();
}