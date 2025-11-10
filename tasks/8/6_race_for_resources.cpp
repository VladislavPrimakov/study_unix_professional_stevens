#include <apue.h>

static void charatatime(const std::string&);

int main(void) {
	pid_t pid;
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid == 0) {
		charatatime("from child process\n");
	}
	else {
		charatatime("from parent process\n");
	}
	return 0;
}

void charatatime(const std::string& str) {
	setbuf(stdout, NULL); /* set unbuffered */
	for (auto& ch : str)
		putc(ch, stdout);
}