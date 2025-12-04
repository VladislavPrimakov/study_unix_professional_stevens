#include "apue.h"
#include <charconv>
#include <liburing.h>

constexpr int QUEUE_DEPTH = 256;
constexpr int BUFLEN = 128;

enum class OpType {
	RECIEVE
};

struct Request {
	OpType type;
	struct sockaddr_in client_addr;
	struct msghdr msg;
	struct iovec iov;
	char buf[BUFLEN];
};

void queue_recv(struct io_uring* ring, int server_fd) {
	auto* req = new Request{};
	req->type = OpType::RECIEVE;
	req->iov.iov_base = req->buf;
	req->iov.iov_len = BUFLEN;
	memset(&req->msg, 0, sizeof(req->msg));
	req->msg.msg_name = &req->client_addr;
	req->msg.msg_namelen = sizeof(req->client_addr);
	req->msg.msg_iov = &req->iov;
	req->msg.msg_iovlen = 1;
	struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
	io_uring_prep_recvmsg(sqe, server_fd, &req->msg, 0);
	io_uring_sqe_set_data(sqe, req);
}

void handle_client_fork(struct sockaddr_in* cliaddr, int server_port) {
	pid_t pid;
	if ((pid = fork()) < 0) {
		syslog(LOG_ERR, "call fork: %s", strerror(errno));
	}
	else if (pid == 0) { // CHILD
		int new_clfd = connect_ipv4_addr(cliaddr, server_port, SOCK_DGRAM);
		if (new_clfd < 0) {
			syslog(LOG_ERR, "child connect failed");
		}
		if (dup2(new_clfd, STDOUT_FILENO) != STDOUT_FILENO || dup2(new_clfd, STDERR_FILENO) != STDERR_FILENO) {
			syslog(LOG_ERR, "error call dup2");
			exit(1);
		}
		close(new_clfd);
		execl("/usr/bin/uptime", "uptime", (char*)0);
		syslog(LOG_ERR, "unexpected return from exec: %s", strerror(errno));
		exit(1);
	}
}

int main(int argc, char* argv[]) {
	if (argc != 2)
		err_quit("usage: ruptimed");
	const char* port_str = argv[1];
	int server_port = 0;
	auto [ptr, ec] = std::from_chars(port_str, port_str + std::strlen(port_str), server_port);
	if (ec != std::errc() || server_port <= 0 || server_port > 65535) {
		err_quit("invalid port number: {}", port_str);
	}
	daemonize("ruptimed upd");
	if (apue_signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
		syslog(LOG_ERR, "can't ignore SIGCHLD: %s", strerror(errno));
		exit(1);
	}
	int server_fd = setup_socket_ipv4(server_port, SOCK_DGRAM);
	if (server_fd < 0) {
		syslog(LOG_ERR, "failed to init socket: %s", strerror(errno));
		exit(1);
	}
	struct io_uring ring;
	if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
		syslog(LOG_ERR, "io_uring_queue_init failed");
		exit(1);
	}
	syslog(LOG_INFO, "started on port udp/%d", server_port);
	struct io_uring_cqe* cqe;
	do {
		queue_recv(&ring, server_fd);
		io_uring_submit(&ring);
		int ret = io_uring_wait_cqe(&ring, &cqe);
		if (ret < 0) {
			syslog(LOG_ERR, "wait_cqe error: %s", strerror(-ret));
			continue;
		}
		Request* req = static_cast<Request*>(io_uring_cqe_get_data(cqe));
		int res = cqe->res;
		io_uring_cqe_seen(&ring, cqe);
		if (req->type == OpType::RECIEVE) {
			if (res >= 0) {
				handle_client_fork(&req->client_addr, server_port);
			}
			else {
				syslog(LOG_ERR, "receive failed: %s", strerror(-res));
			}
		}
		delete req;
	} while (true);
	io_uring_queue_exit(&ring);
	close(server_fd);
	return 0;
}