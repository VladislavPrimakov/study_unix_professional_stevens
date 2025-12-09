#include "apue.h"

void cat_file(int taken_fd) {
	char buf[MAXLINE];
	while (true) {
		int n = read(taken_fd, buf, MAXLINE);
		if (n == 0) {
			break;
		}
		if (n < 0) {
			err_msg("call read");
			break;
		}
		if (!writen(STDOUT_FILENO, buf, n)) {
			err_msg("call write");
			break;
		}
	}
}

int main(int argc, char* argv[]) {
	char line[UNIX_SOCKET_MAX_MSG_SIZE];
	char welcome[] = "\n> ";
	if (!writen(STDOUT_FILENO, welcome, 3)) {
		err_sys("call write");
	}
	while (fgets(line, UNIX_SOCKET_MAX_MSG_SIZE, stdin) != NULL) {
		size_t len = strlen(line);
		if (len > 0 && line[len - 1] == '\n')
			line[len - 1] = 0;
		int taken_fd = unix_socket_client_open(line, O_RDONLY);
		if (taken_fd >= 0) {
			cat_file(taken_fd);
		}
		if (!writen(STDOUT_FILENO, welcome, 3)) {
			err_sys("call write");
		}
	}
	return 0;
}