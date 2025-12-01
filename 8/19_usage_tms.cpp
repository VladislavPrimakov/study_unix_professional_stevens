#include "apue.h"
#include <sys/times.h>

void pr_times(clock_t, struct tms*, struct tms*);
void do_cmd(char*);

int main(int argc, char* argv[]) {
	int i;
	setbuf(stdout, NULL);
	for (i = 1; i < argc; i++)
		do_cmd(argv[i]);
	exit(0);
}

void do_cmd(char* cmd) {
	struct tms tmsstart, tmsend;
	clock_t start, end;
	int status;
	std::println("\ncommand: {}", cmd);
	if ((start = times(&tmsstart)) == -1)
		err_sys("call times");
	if ((status = system(cmd)) < 0)
		err_sys("call system()");
	if ((end = times(&tmsend)) == -1)
		err_sys("call times");
	pr_times(end - start, &tmsstart, &tmsend);
	pr_exit(status);
}

void pr_times(clock_t real, struct tms* tmsstart, struct tms* tmsend) {
	static long clktck = 0;
	if (clktck == 0)
		if ((clktck = sysconf(_SC_CLK_TCK)) < 0)
			err_sys("call sysconf");
	std::println(" real: {:7.2}", real / (double)clktck);
	std::println(" user: {:7.2}", (tmsend->tms_utime - tmsstart->tms_utime) / (double)clktck);
	std::println(" sys: {:7.2}", (tmsend->tms_stime - tmsstart->tms_stime) / (double)clktck);
	std::println(" child user: {:7.2}", (tmsend->tms_cutime - tmsstart->tms_cutime) / (double)clktck);
	std::println(" child sys: {:7.2}", (tmsend->tms_cstime - tmsstart->tms_cstime) / (double)clktck);
}