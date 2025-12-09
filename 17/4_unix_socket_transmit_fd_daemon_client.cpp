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
	if (argc != 3) {
		err_quit("Usage {} <pathname> <mode>", argv[0]);
	}
	const char* path = argv[1];
	int mode = std::stoi(argv[2]); // 0 (O_RDONLY)

	// connect to daemon via unix domain socket
	int server_fd = unix_socket_cli_conn(UNIX_SOCKET_CS_OPEN);
	if (server_fd < 0) {
		err_sys("call unix_socket_cli_conn");
	}
	if (unix_socket_send_request(server_fd, UNIX_SOCKET_COMMAND::OPEN, path, mode) < 0) {
		err_quit("call unix_socket_send_request");
	}
	// get FD from daemon
	int taken_fd = unix_socket_recv_fd(server_fd, NULL);
	if (taken_fd >= 0) {
		cat_file(taken_fd);
		close(taken_fd);
	}
	return 0;
}