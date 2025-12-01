#include "apue.h"

int main() {
	int n;
	int fd[2];
	pid_t pid;
	char line[MAXLINE];
	if (pipe(fd) < 0)
		err_sys("call pipe");
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid > 0) { // parent
		close(fd[0]);
		int res = write(fd[1], "Hello world\n", 12);
	}
	else { // child
		close(fd[1]);
		n = read(fd[0], line, MAXLINE);
		int res = write(STDOUT_FILENO, line, n);
	}
	exit(0);
}