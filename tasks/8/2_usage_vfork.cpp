#include "apue.h"
int globvar = 6;

int main(void) {
	int var;
	pid_t pid;
	var = 88;
	std::println("before fork");
	if ((pid = vfork()) < 0) {
		err_sys("call vfork");
	}
	else if (pid == 0) {
		globvar++;
		var++;
		_exit(0);
	}

	std::println("pid = {}, globvar = {}, var = {}", getpid(), globvar, var);
	return 0;
}