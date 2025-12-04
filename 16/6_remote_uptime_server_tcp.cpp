#include "apue.h"
#include <charconv>
#include <liburing.h>

constexpr int QUEUE_DEPTH = 256;

enum class OpType {
	ACCEPT
};

struct Request {
	OpType type;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
};

void queue_accept(struct io_uring* ring, int server_fd) {
	auto* req = new Request{};
	req->type = OpType::ACCEPT;
	struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
	io_uring_prep_accept(sqe, server_fd, (struct sockaddr*)&req->client_addr, &req->client_addr_len, 0);
	io_uring_sqe_set_data(sqe, req);
}

void handle_client_fork(int clfd) {
	pid_t pid;
	if ((pid = fork()) < 0) {
		syslog(LOG_ERR, "call fork: %s", strerror(errno));
		close(clfd);
	}
	else if (pid == 0) { // CHILD
		if (dup2(clfd, STDOUT_FILENO) != STDOUT_FILENO || dup2(clfd, STDERR_FILENO) != STDERR_FILENO) {
			syslog(LOG_ERR, "error call dup2");
			exit(1);
		}
		close(clfd);
		execl("/usr/bin/uptime", "uptime", (char*)0);
		syslog(LOG_ERR, "unexpected return from exec: %s", strerror(errno));
		exit(1);
	}
	else { // PARENT
		close(clfd);
	}
}

int main(int argc, char* argv[]) {
	if (argc != 2)
		err_quit("usage: ruptimed port");
	const char* port_str = argv[1];
	int server_port = 0;
	auto [ptr, ec] = std::from_chars(port_str, port_str + std::strlen(port_str), server_port);
	if (ec != std::errc() || server_port <= 0 || server_port > 65535) {
		err_quit("invalid port number: {}", port_str);
	}
	daemonize("ruptimed tcp");
	if (apue_signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
		syslog(LOG_ERR, "can't ignore SIGCHLD: %s", strerror(errno));
		exit(1);
	}
	int server_fd = setup_socket_ipv4(server_port, SOCK_STREAM);
	if (server_fd < 0) {
		syslog(LOG_ERR, "failed to init socket: %s", strerror(errno));
		exit(1);
	}
	if (listen(server_fd, 10) < 0) {
		close(server_fd);
		return -1;
	}
	struct io_uring ring;
	if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
		syslog(LOG_ERR, "io_uring_queue_init failed");
		exit(1);
	}
	syslog(LOG_INFO, "started on port tcp/%d", server_port);
	struct io_uring_cqe* cqe;
	do {
		queue_accept(&ring, server_fd);
		io_uring_submit(&ring);
		int ret = io_uring_wait_cqe(&ring, &cqe);
		if (ret < 0) {
			syslog(LOG_ERR, "wait_cqe error: %s", strerror(-ret));
			continue;
		}
		Request* req = static_cast<Request*>(io_uring_cqe_get_data(cqe));
		int res = cqe->res;
		io_uring_cqe_seen(&ring, cqe);
		if (req->type == OpType::ACCEPT) {
			if (res >= 0) {
				int client_fd = res;
				handle_client_fork(client_fd);
			}
			else {
				syslog(LOG_ERR, "accept failed: %s", strerror(-res));
			}
		}
		delete req;
	} while (true);
	io_uring_queue_exit(&ring);
	close(server_fd);
	return 0;
}