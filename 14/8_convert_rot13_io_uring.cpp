#include "apue.h"
#include <liburing.h>

#define BSZ 4096
#define QUEUE_DEPTH  16

off_t global_read_offset = 0;
int pending_operations = 0;
bool eof_reached = false;

unsigned char translate(unsigned char c) {
	if (isalpha(c)) {
		if (c >= 'n')
			c -= 13;
		else if (c >= 'a')
			c += 13;
		else if (c >= 'N')
			c -= 13;
		else
			c += 13;
	}
	return(c);
}

enum OpType {
	OP_READ,
	OP_WRITE
};

struct Request {
	int id;
	OpType type;
	unsigned char buf[BSZ];
	off_t offset;
	int iovec_len;
};

void queue_read(struct io_uring* ring, int fd, Request* req) {
	if (eof_reached) return;
	struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
	if (!sqe)
		err_sys("queue_read: get sqe failed");
	req->type = OP_READ;
	req->offset = global_read_offset;
	global_read_offset += BSZ;
	io_uring_prep_read(sqe, fd, req->buf, BSZ, req->offset);
	io_uring_sqe_set_data(sqe, req);
	pending_operations++;
}

void queue_write(struct io_uring* ring, int fd, Request* req) {
	struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
	if (!sqe)
		err_sys("queue_write: get sqe failed");
	req->type = OP_WRITE;
	io_uring_prep_write(sqe, fd, req->buf, req->iovec_len, req->offset);
	io_uring_sqe_set_data(sqe, req);
	pending_operations++;
}

int main(int argc, char* argv[]) {
	int ifd, ofd, i, n, nw;
	if (argc != 3)
		err_quit("Usage: {} infile outfile", argv[0]);
	if ((ifd = open(argv[1], O_RDONLY)) < 0)
		err_sys("call open {}", argv[1]);
	if ((ofd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, FILE_MODE)) < 0)
		err_sys("call create {}", argv[2]);

	struct io_uring ring;
	if (io_uring_queue_init(QUEUE_DEPTH, &ring, IORING_SETUP_SQPOLL) < 0) {
		std::println(std::cerr, "io_uring_queue_init(IORING_SETUP_SQPOLL) failed (check sudo or ulimit -l)");
	}
	else {
		if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0)
			err_sys("io_uring_queue_init");
	}

	std::vector<Request> requests(QUEUE_DEPTH);
	for (int i = 0; i < QUEUE_DEPTH; ++i) {
		requests[i].id = i;
		queue_read(&ring, ifd, &requests[i]);
	}
	io_uring_submit(&ring);

	while (pending_operations > 0) {
		struct io_uring_cqe* cqe;
		int ret = io_uring_wait_cqe(&ring, &cqe);
		if (ret < 0)
			err_sys("wait_cqe");
		Request* req = (Request*)io_uring_cqe_get_data(cqe);
		int res = cqe->res;
		io_uring_cqe_seen(&ring, cqe);
		pending_operations--;

		if (res < 0) {
			err_exit(res, "Async operation failed");
		}
		switch (req->type) {
		case OP_READ:
			// if not EOF
			if (res > 0) {
				req->iovec_len = res;
				if (res < BSZ) eof_reached = true;
				for (int i = 0; i < res; ++i) {
					req->buf[i] = translate(req->buf[i]);
				}
				queue_write(&ring, ofd, req);
			}
			else {
				eof_reached = true;
			}
			break;
		case OP_WRITE:
			if (res != req->iovec_len) {
				err_quit("written less than read ({}/{})", res, req->iovec_len);
			}
			queue_read(&ring, ifd, req);
			break;
		}
		io_uring_submit(&ring);
	}
	fsync(ofd);
	close(ifd);
	close(ofd);
	io_uring_queue_exit(&ring);
	return 0;
}