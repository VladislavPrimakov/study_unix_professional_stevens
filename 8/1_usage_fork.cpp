#include "apue.h"

int globvar = 6;
std::string buf = "record to stdout\n";

int main(void) {
	int var;
	pid_t pid;
	var = 88;
	if (!writen(STDOUT_FILENO, buf.data(), buf.size()))
		err_sys("call write");
	std::println("before fork");
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid == 0) {
		globvar++;
		var++;
	}
	else {
		sleep(2);
	}
	std::println("pid = {}, globvar = {}, var = {}", getpid(), globvar, var);
	return 0;
}