#include "apue.h"
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
		if (writen(STDOUT_FILENO, buf, n_read) != n_read) {
			err_sys("write error");
		}
	}
}

int main(int argc, char* argv[]) {
	if (argc != 2)
		err_quit("usage: {} hostname", argv[0]);
	struct io_uring ring;
	if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
		err_sys("io_uring_queue_init failed");
	}
	std::println(stderr, "Connecting to {} on port 4000...", argv[1]);
	int sockfd = connect_to_server(argv[1], 4000, SOCK_STREAM, 5);
	if (sockfd < 0) {
		err_exit(errno, "can't connect to {}", argv[1]);
	}
	download_uptime(&ring, sockfd);
	close(sockfd);
	io_uring_queue_exit(&ring);
	return 0;
}