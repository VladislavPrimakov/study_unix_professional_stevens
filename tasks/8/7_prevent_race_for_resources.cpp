#include "apue.h"

static void charatatime(const std::string&);

int main(void) {
	pid_t pid;
	TELL_WAIT();
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid == 0) {
		WAIT_PARENT();
		charatatime("from child process\n");
	}
	else {
		charatatime("from parent process\n");
		TELL_CHILD(pid);
	}
	return 0;
}

void charatatime(const std::string& str) {
	setbuf(stdout, NULL); /* set unbuffered */
	for (auto& ch : str)
		putc(ch, stdout);
}