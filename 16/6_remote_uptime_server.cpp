#include "apue.h"
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
		syslog(LOG_ERR, "ruptimed: call fork: %s", strerror(errno));
		close(clfd);
	}
	else if (pid == 0) { // CHILD
		if (dup2(clfd, STDOUT_FILENO) != STDOUT_FILENO || dup2(clfd, STDERR_FILENO) != STDERR_FILENO) {
			syslog(LOG_ERR, "ruptimed: error call dup2");
			exit(1);
		}
		close(clfd);
		execl("/usr/bin/uptime", "uptime", (char*)0);
		syslog(LOG_ERR, "ruptimed: unexpected return from exec: %s", strerror(errno));
		exit(1);
	}
	else { // PARENT
		close(clfd);
	}
}

int main(int argc, char* argv[]) {
	if (argc != 1)
		err_quit("usage: ruptimed");
	daemonize("ruptimed");
	if (apue_signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
		syslog(LOG_ERR, "ruptimed: can't ignore SIGCHLD: %s", strerror(errno));
		exit(1);
	}
	int server_fd = setup_server_ipv4(4000, SOCK_STREAM, 10);
	if (server_fd < 0) {
		syslog(LOG_ERR, "ruptimed: failed to init socket: %s", strerror(errno));
		exit(1);
	}

	struct io_uring ring;
	if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
		syslog(LOG_ERR, "ruptimed: io_uring_queue_init failed");
		exit(1);
	}

	queue_accept(&ring, server_fd);
	io_uring_submit(&ring);

	syslog(LOG_INFO, "ruptimed started on port 4000");

	while (true) {
		struct io_uring_cqe* cqe;
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
		queue_accept(&ring, server_fd);
		io_uring_submit(&ring);
	}

	io_uring_queue_exit(&ring);
	close(server_fd);
	return 0;
}