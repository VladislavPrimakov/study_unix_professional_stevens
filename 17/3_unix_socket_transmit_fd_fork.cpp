#include "apue.h"

void cat_fixed_file(struct io_uring* ring, int fixed_idx) {
	off_t offset = 0;
	char buf[MAXLINE];
	struct io_uring_sqe* sqe;
	struct io_uring_cqe* cqe;
	while (true) {
		sqe = io_uring_get_sqe(ring);
		if (!sqe)
			err_quit("call io_uring_get_sqe");

		io_uring_prep_read_fixed(sqe, fixed_idx, buf, MAXLINE, offset, 0);
		io_uring_submit(ring);

		if (io_uring_wait_cqe(ring, &cqe) < 0)
			err_sys("client read wait");

		int n = cqe->res;
		io_uring_cqe_seen(ring, cqe);

		if (n < 0) {
			std::println(stderr, "Read error: {}", strerror(-n));
			break;
		}
		if (n == 0)
			break;

		if (!writen(STDOUT_FILENO, buf, n))
			err_sys("call write");
		offset += n;
	}
}

int main(int argc, char* argv[]) {
	struct io_uring ring;
	if (io_uring_queue_init(UNIX_SOCKET_QUEUE_DEPTH, &ring, 0) < 0)
		err_sys("io_uring_queue_init");

	char line[MAXLINE];
	while (fgets(line, MAXLINE, stdin) != NULL) {
		size_t len = strlen(line);
		if (len > 0 && line[len - 1] == '\n')
			line[len - 1] = 0;

		int fixed_idx = unix_socket_client_open(&ring, line, O_RDONLY);
		if (fixed_idx < 0) {
			std::println(stderr, "Cannot open {}: {}", line, strerror(errno));
		}
		else {
			cat_fixed_file(&ring, fixed_idx);
		}
	}
	io_uring_queue_exit(&ring);
	return 0;
}