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
		std::string msg = "Hello world\n";
		if (!writen(fd[1], msg.data(), msg.size())) {
			err_sys("call write");
		}
	}
	else { // child
		close(fd[1]);
		n = read(fd[0], line, MAXLINE);
		if (!writen(STDOUT_FILENO, line, n)) {
			err_sys("call write");
		}
	}
	exit(0);
}