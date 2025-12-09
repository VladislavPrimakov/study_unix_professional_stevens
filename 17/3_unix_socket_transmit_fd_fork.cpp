#include "apue.h"

void unix_socket_server_loop(int socket_fd) {
	if (unix_socket_check_domain(socket_fd) < 0) {
		return;
	}
	char buf[UNIX_SOCKET_MAX_MSG_SIZE];
	while (true) {
		StatusMsg status;
		// read request from socket
		int n = read(socket_fd, buf, sizeof(buf));
		if (n == 0) {
			break; // Client closed
		}
		if (n < 0) {
			err_ret("[Server] read {}", socket_fd);
			break;
		}
		unix_socket_process_request(socket_fd, std::string(buf, n));
	}
}

int unix_socket_client_open(const char* path, mode_t mode) {
	int sockfd[2] = { -1, -1 };
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) < 0)
		err_sys("call socketpair");
	pid_t pid = fork();
	if (pid < 0) {
		close(sockfd[0]); close(sockfd[1]);
		err_sys("call fork");
	}
	if (pid == 0) { // Child (Server): read sockfd[1] for requests
		close(sockfd[0]);
		unix_socket_server_loop(sockfd[1]);
		close(sockfd[1]);
		exit(0);
	}
	// Parent (Client): send requests to sockdf[0] and wait for msg with fd 
	close(sockfd[1]);
	if (unix_socket_send_request(sockfd[0], UNIX_SOCKET_COMMAND::OPEN, path, mode) < 0) {
		return -1;
	}
	int fd = unix_socket_recv_fd(sockfd[0], NULL);
	close(sockfd[0]);
	waitpid(pid, NULL, 0);
	return fd;
}

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
			close(taken_fd);
		}
		if (!writen(STDOUT_FILENO, welcome, 3)) {
			err_sys("call write");
		}
	}
	return 0;
}