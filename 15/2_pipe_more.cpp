#include "apue.h"

constexpr const char* DEF_PAGER = "/bin/more";

int main(int argc, char* argv[]) {
	int n;
	int fd[2];
	pid_t pid;
	const char* pager;
	const char* argv0;
	char line[MAXLINE];
	FILE* fp;
	if (argc != 2)
		err_quit("Usage {}: <pathname>", argv[0]);
	if ((fp = fopen(argv[1], "r")) == NULL)
		err_sys("can't open {}", argv[1]);
	if (pipe(fd) < 0)
		err_sys("call pipe");
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid > 0) { // parent
		close(fd[0]);
		while (fgets(line, MAXLINE, fp) != NULL) {
			n = strlen(line);
			if (writen(fd[1], line, n) != n)
				err_sys("call write to pipe");
		}
		if (ferror(fp))
			err_sys("call fgets");
		close(fd[1]);
		if (waitpid(pid, NULL, 0) < 0)
			err_sys("call waitpid");
		exit(0);
	}
	else { // child
		close(fd[1]);
		if (fd[0] != STDIN_FILENO) {
			if (dup2(fd[0], STDIN_FILENO) != STDIN_FILENO)
				err_sys("call dup2 from pipe to stdin");
			close(fd[0]);
		}
		if ((pager = getenv("PAGER")) == NULL)
			pager = DEF_PAGER;
		if ((argv0 = strrchr(pager, '/')) != NULL)
			argv0++;
		else
			argv0 = pager;
		if (execl(pager, argv0, (char*)0) < 0)
			err_sys("call exec {}", pager);
	}
	exit(0);
}