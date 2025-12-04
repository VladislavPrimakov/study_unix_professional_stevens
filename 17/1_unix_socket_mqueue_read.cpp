#include "apue.h"
#include <liburing.h>
#include <mqueue.h>

constexpr int NQ = 3;           // number of queues
constexpr int MAXMSZ = 512;     // maximum message size
constexpr char QUEUE_NAME_FMT[] = "/test_mq_%d";
constexpr int RING_DEPTH = 8;
int main_read_fds[NQ];			// Store read-side FDs globally or in main to re-access them
char read_buffers[NQ][MAXMSZ];	//  Buffers for io_uring to read into. 

struct threadinfo {
	mqd_t qid;
	int fd;
	int index;
};

// receive message from queue and write to socket
void* thr_fun(void* arg) {
	int n;
	struct threadinfo* tip = (struct threadinfo*)arg;
	char buf[MAXMSZ];
	while (true) {
		memset(buf, 0, sizeof(buf));
		if ((n = mq_receive(tip->qid, buf, MAXMSZ, 0)) < 0)
			err_sys("call mq_receive");
		if (!writen(tip->fd, buf, n))
			err_sys("call write");
	}
	return nullptr;
}

// prepare a read request to ring
void queue_read(struct io_uring* ring, int fd, int index) {
	struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
	if (!sqe)
		err_sys("error getting sqe");
	io_uring_prep_read(sqe, fd, read_buffers[index], MAXMSZ, 0);
	io_uring_sqe_set_data64(sqe, (uint64_t)index);
}

int main() {
	int i, err;
	int fd[2];
	mqd_t qid[NQ];
	struct threadinfo ti[NQ];
	pthread_t tid[NQ];
	struct io_uring ring;
	if (io_uring_queue_init(RING_DEPTH, &ring, 0) < 0)
		err_sys("call io_uring_queue_init");
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = MAXMSZ;
	attr.mq_curmsgs = 0;

	for (i = 0; i < NQ; i++) {
		char name[32];
		sprintf(name, QUEUE_NAME_FMT, i);
		mq_unlink(name); // Cleanup old queues
		if ((qid[i] = mq_open(name, O_RDWR | O_CREAT, 0666, &attr)) == (mqd_t)-1)
			err_sys("call mq_open");
		std::println("queue {} opened with descriptor {}", i, (int)qid[i]);
		if (socketpair(AF_UNIX, SOCK_DGRAM, 0, fd) < 0)
			err_sys("call socketpair");
		main_read_fds[i] = fd[0]; // Save for re-submission
		ti[i].qid = qid[i];
		ti[i].fd = fd[1];
		ti[i].index = i;
		queue_read(&ring, main_read_fds[i], i);
		if ((err = pthread_create(&tid[i], NULL, thr_fun, &ti[i])) != 0)
			err_exit(err, "call pthread_create");
	}

	io_uring_submit(&ring);
	std::println("Main loop started.");

	while (true) {
		struct io_uring_cqe* cqe;
		int ret = io_uring_wait_cqe(&ring, &cqe);
		if (ret < 0)
			err_sys("call io_uring_wait_cqe");
		int queue_idx = (int)io_uring_cqe_get_data64(cqe);
		int res = cqe->res;
		io_uring_cqe_seen(&ring, cqe);
		if (res > 0) {
			if (res < MAXMSZ)
				read_buffers[queue_idx][res] = 0;
			else
				read_buffers[queue_idx][MAXMSZ - 1] = 0;
			std::println("queue: {}, message: {}", queue_idx, read_buffers[queue_idx]);
		}
		else if (res < 0) {
			err_cont(-res, "read error on queue {}", queue_idx);
		}
		queue_read(&ring, main_read_fds[queue_idx], queue_idx);
		io_uring_submit(&ring);
	}
}