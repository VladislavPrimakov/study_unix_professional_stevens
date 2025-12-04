#include "apue.h"
#include <charconv>
#include <liburing.h>

constexpr int QUEUE_DEPTH = 8;
constexpr size_t BUFLEN = 128;

void download_uptime(struct io_uring* ring, int sockfd) {
	char buf[BUFLEN];
	struct io_uring_sqe* sqe;
	struct io_uring_cqe* cqe;
	while (true) {
		sqe = io_uring_get_sqe(ring);
		if (!sqe)
			err_sys("get sqe recv");
		io_uring_prep_recv(sqe, sockfd, buf, BUFLEN, 0);
		io_uring_submit(ring);
		if (io_uring_wait_cqe(ring, &cqe) < 0)
			err_sys("wait recv");
		int n_read = cqe->res;
		io_uring_cqe_seen(ring, cqe);
		if (n_read < 0) {
			err_exit(-n_read, "recv error: {}");
		}
		else if (n_read == 0) { // server closed connection
			break;
		}
		if (!writen(STDOUT_FILENO, buf, n_read)) {
			err_sys("write error");
		}
	}
}

int main(int argc, char* argv[]) {
	if (argc != 3)
		err_quit("usage: {} hostname port", argv[0]);
	const char* port_str = argv[2];
	int server_port = 0;
	auto [ptr, ec] = std::from_chars(port_str, port_str + std::strlen(port_str), server_port);
	if (ec != std::errc() || server_port <= 0 || server_port > 65535) {
		err_quit("invalid port number: {}", port_str);
	}
	struct io_uring ring;
	if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
		err_sys("io_uring_queue_init failed");
	}
	std::println("Connecting to {} on port tcp/{}...", argv[1], server_port);
	int sockfd = connect_ipv4_host(argv[1], server_port, SOCK_STREAM);
	if (sockfd < 0) {
		err_exit(errno, "can't connect to {}", argv[1]);
	}
	download_uptime(&ring, sockfd);
	close(sockfd);
	io_uring_queue_exit(&ring);
	return 0;
}