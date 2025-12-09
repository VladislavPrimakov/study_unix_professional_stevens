#include "apue.h"

enum OpType {
	OP_ACCEPT,
	OP_READ
};

struct Request {
	OpType type;
	int client_fd;
	char buffer[UNIX_SOCKET_MAX_MSG_SIZE];
	struct sockaddr_un client_addr;
	socklen_t client_len;
};

struct io_uring ring;
int listen_fd;

void add_accept_request(int lfd) {
	struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
	Request* req = new Request;
	req->type = OP_ACCEPT;
	req->client_len = sizeof(req->client_addr);
	io_uring_prep_accept(sqe, lfd, (struct sockaddr*)&req->client_addr, &req->client_len, 0);
	io_uring_sqe_set_data(sqe, req);
}

void add_read_request(int cfd) {
	struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
	Request* req = new Request;
	req->type = OP_READ;
	req->client_fd = cfd;
	io_uring_prep_read(sqe, cfd, req->buffer, UNIX_SOCKET_MAX_MSG_SIZE, 0);
	io_uring_sqe_set_data(sqe, req);
}

void handle_client_request(int client_fd, char* buf, int len) {
	if (len > 0) {
		unix_socket_process_request(client_fd, std::string(buf, len));
	}
}

int main() {
	int queue_depth = 10;
	daemonize("unix socket daemon");
	listen_fd = unix_socket_serv_listen(UNIX_SOCKET_CS_OPEN, queue_depth);
	if (listen_fd < 0) {
		err_sys("call unix_socket_serv_listen");
	}
	if (io_uring_queue_init(queue_depth, &ring, 0) < 0) {
		err_sys("call io_uring_queue_init");
	}
	syslog(LOG_INFO, "Started listening on: %s", UNIX_SOCKET_CS_OPEN);
	do {
		add_accept_request(listen_fd);
		io_uring_submit(&ring);
		struct io_uring_cqe* cqe;
		int ret = io_uring_wait_cqe(&ring, &cqe);
		if (ret < 0) {
			err_msg("call io_uring_wait_cqe");
			break;
		}
		Request* req = (Request*)io_uring_cqe_get_data(cqe);
		int res = cqe->res;
		switch (req->type) {
		case OP_ACCEPT:
			if (res >= 0) {
				int client_fd = res;
				syslog(LOG_INFO, "New client connected %d", client_fd);
				add_read_request(client_fd);
				add_accept_request(listen_fd);
			}
			else {
				err_cont(-res, "Accept failed");
			}
			break;
		case OP_READ:
			if (res > 0) {
				handle_client_request(req->client_fd, req->buffer, res);
				syslog(LOG_INFO, "Request handled %d", req->client_fd);
				close(req->client_fd);
			}
			else {
				// Client disconnected (res == 0) or error (res < 0)
				close(req->client_fd);
			}
			break;
		}
		delete req;
		io_uring_cqe_seen(&ring, cqe);
		io_uring_submit(&ring);
	} while (true);
	close(listen_fd);
	io_uring_queue_exit(&ring);
	unlink(UNIX_SOCKET_CS_OPEN);
	return 0;
}