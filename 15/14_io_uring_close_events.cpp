//Объясните, как функции select и poll обрабатывают ситуацию закрытия неименованного канала пишущим процессом.
//Чтобы ответить на этот вопрос, напишите две небольшие программы : одну — с использованием функции select, другую — с использованием функции poll.
//Повторите это упражнение для проверки ситуации, когда канал закрывается читающим процессом.

#include "apue.h"
#include <liburing.h>
#include <poll.h>

void print_poll_events(int events) {
	std::string s;
	if (events & POLLIN)
		s += " POLLIN";
	if (events & POLLOUT)
		s += " POLLOUT";
	if (events & POLLHUP)
		s += " POLLHUP";
	if (events & POLLERR)
		s += " POLLERR";
	if (events & POLLRDHUP)
		s += " POLLRDHUP";
	std::println("{}", s);
}

int main() {
	struct io_uring ring;
	if (io_uring_queue_init(8, &ring, 0) < 0) {
		err_sys("call io_uring_queue_init");
	}
	int fd[2];

	std::println("\n--- Test io_uring: Writer closes pipe ---\n");

	if (pipe(fd) < 0) err_sys("call pipe");
	if (fork() == 0) {
		// Child (Writer)
		close(fd[0]);
		sleep(1);
		std::println("[Child] Close write...");
		close(fd[1]);
		exit(0);
	}

	// Parent (Reader)
	close(fd[1]);

	struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
	if (!sqe)
		err_sys("get sqe");
	io_uring_prep_poll_add(sqe, fd[0], POLLIN | POLLRDHUP);
	io_uring_sqe_set_data(sqe, (void*)(uintptr_t)fd[0]);
	std::println("[Parent] io_uring_submit_and_wait...");
	struct io_uring_cqe* cqe;
	if (io_uring_submit_and_wait(&ring, 1) < 0)
		err_sys("submit_and_wait");
	if (io_uring_peek_cqe(&ring, &cqe) == 0) {
		int events = cqe->res;
		int event_fd = (int)(uintptr_t)io_uring_cqe_get_data(cqe);
		if (events < 0) {
			err_ret("Poll error");
		}
		else {
			std::print("[Parent] Event for fd {}:", event_fd);
			print_poll_events(events);
		}
		io_uring_cqe_seen(&ring, cqe);
	}
	close(fd[0]);

	std::println("\n--- Test io_uring: Reader closes pipe ---\n");
	signal(SIGPIPE, SIG_IGN);
	if (pipe(fd) < 0)
		err_sys("call pipe");
	if (fork() == 0) {
		// Child (Reader)
		close(fd[1]);
		sleep(1);
		std::println("[Child] Close read...");
		close(fd[0]);
		exit(0);
	}
	// Parent (Writr)
	close(fd[0]);
	sleep(2);
	sqe = io_uring_get_sqe(&ring);
	if (!sqe) err_sys("get sqe");
	io_uring_prep_poll_add(sqe, fd[1], POLLOUT);
	io_uring_sqe_set_data(sqe, (void*)(uintptr_t)fd[1]);
	std::println("[Parent] io_uring_submit_and_wait...");
	if (io_uring_submit_and_wait(&ring, 1) < 0)
		err_sys("submit_and_wait");
	if (io_uring_peek_cqe(&ring, &cqe) == 0) {
		int events = cqe->res;
		int event_fd = (int)(uintptr_t)io_uring_cqe_get_data(cqe);
		if (events < 0) {
			err_ret("Poll error");
		}
		else {
			std::print("[Parent] Event for fd {}:", event_fd);
			print_poll_events(events);
		}
		io_uring_cqe_seen(&ring, cqe);
	}
	close(fd[1]);
	io_uring_queue_exit(&ring);

	return 0;
}