#include "apue.h"

int main() {
	int n, int1, int2;
	char line[MAXLINE];

	while ((n = read(STDIN_FILENO, line, MAXLINE)) > 0) {
		line[n] = 0;
		if (sscanf(line, "%d%d", &int1, &int2) == 2) {
			sprintf(line, "%d\n", int1 + int2);
			n = strlen(line);
			if (writen(STDOUT_FILENO, line, n) != n)
				err_sys("call writen");
		}
		else {
			char* msg = "invalid arguments\n";
			if (writen(STDOUT_FILENO, msg, strlen(msg)) < 0)
				err_sys("call writen");
		}
	}
	exit(0);
}