#include "apue.h"

void sig_pipe(int signo) {
	printf("taken SIGPIPE\n");
	exit(1);
}

int main() {
	int n, fd1[2], fd2[2];
	pid_t pid;
	char line[MAXLINE];
	if (apue_signal(SIGPIPE, sig_pipe) == SIG_ERR)
		err_sys("call signal");
	if (pipe(fd1) < 0 || pipe(fd2) < 0)
		err_sys("call pipe");
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid > 0) { // PARENT
		close(fd1[0]);
		close(fd2[1]);
		while (fgets(line, MAXLINE, stdin) != NULL) {
			n = strlen(line);
			if (!writen(fd1[1], line, n))
				err_sys("call write to pipe");
			if ((n = read(fd2[0], line, MAXLINE)) < 0)
				err_sys("call read from pipe");
			if (n == 0) {
				err_msg("pipe was closed in child process");
				break;
			}
			line[n] = 0;
			if (fputs(line, stdout) == EOF)
				err_sys("fputs error");
		}
		if (ferror(stdin))
			err_sys("call fgets");
		exit(0);
	}
	else { // CHILD
		close(fd1[1]);
		close(fd2[0]);
		if (fd1[0] != STDIN_FILENO) {
			if (dup2(fd1[0], STDIN_FILENO) != STDIN_FILENO)
				err_sys("call dup2 for stdin");
			close(fd1[0]);
		}
		if (fd2[1] != STDOUT_FILENO) {
			if (dup2(fd2[1], STDOUT_FILENO) != STDOUT_FILENO)
				err_sys("call dup2 for stdout");
			close(fd2[1]);
		}
		if (execl("./15_8_filter_add_2", "15_8_filter_add_2", (char*)0) < 0)
			err_sys("call exec");
	}
	exit(0);
}
