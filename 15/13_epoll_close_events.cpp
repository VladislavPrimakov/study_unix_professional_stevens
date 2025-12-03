//Объясните, как функции select и poll обрабатывают ситуацию закрытия неименованного канала пишущим процессом.
//Чтобы ответить на этот вопрос, напишите две небольшие программы : одну — с использованием функции select, другую — с использованием функции poll.
//Повторите это упражнение для проверки ситуации, когда канал закрывается читающим процессом.

#include <apue.h>
#include <sys/epoll.h>

void print_events(uint32_t events) {
	std::string s;
	if (events & EPOLLIN)
		s += " EPOLLIN";
	if (events & EPOLLOUT)
		s += " EPOLLOUT";
	if (events & EPOLLHUP)
		s += " EPOLLHUP";
	if (events & EPOLLERR)
		s += " EPOLLERR";
	std::println("{}", s);
}

int main() {
	int fd[2];
	if (pipe(fd) < 0) {
		err_sys("call pipe");
	}
	int epfd = epoll_create1(0);
	if (epfd < 0) {
		err_sys("epoll_create1");
	}
	struct epoll_event ev, events[1];

	std::println("\n--- Test epoll: Writer closes pipe ---\n");
	if (fork() == 0) {
		// Child (Writer)
		close(fd[0]);
		sleep(1);
		std::println("[Child] Close write...");
		close(fd[1]);
		exit(0);
	}
	close(fd[1]);
	ev.events = EPOLLIN | EPOLLRDHUP;
	ev.data.fd = fd[0];
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd[0], &ev) < 0) {
		err_sys("call epoll_ctl(EPOLL_CTL_ADD)");
	}
	std::println("[Parent] epoll_wait...");
	int nfds = epoll_wait(epfd, events, 1, -1);
	if (nfds < 0) {
		err_sys("call epoll_wait");
	}
	else if (nfds > 0) {
		std::print("[Parent] Event for fd {}:", static_cast<int>(events[0].data.fd));
		print_events(events[0].events);
	}
	close(fd[0]);
	close(epfd);

	std::println("\n--- Test epoll: Reader closes pipe ---\n");
	signal(SIGPIPE, SIG_IGN);
	if (pipe(fd) < 0) {
		err_sys("call pipe");
	}
	epfd = epoll_create1(0);
	if (epfd < 0) {
		err_sys("epoll_create1");
	}
	if (fork() == 0) {
		// Child (Reader)
		close(fd[1]);
		sleep(1);
		std::println("[Child] Close read...");
		close(fd[0]);
		exit(0);
	}
	// Parent (Writer)
	close(fd[0]);
	ev.events = EPOLLOUT;
	ev.data.fd = fd[1];
	epoll_ctl(epfd, EPOLL_CTL_ADD, fd[1], &ev);
	sleep(2);
	std::println("[Parent] epoll_wait...");
	nfds = epoll_wait(epfd, events, 1, -1);
	if (nfds < 0) {
		err_sys("call epoll_wait");
	}
	else if (nfds > 0) {
		std::print("[Parent] Event for fd {}:", static_cast<int>(events[0].data.fd));
		print_events(events[0].events);
	}
	close(fd[1]);
	close(epfd);

	return 0;
}