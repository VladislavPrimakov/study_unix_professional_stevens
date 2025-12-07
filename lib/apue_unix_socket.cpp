#include "apue.h"

int unix_socket_serv_listen(const char* name, unsigned int qlen) {
	int fd, len, err, rval;
	struct sockaddr_un un;
	if (strlen(name) >= sizeof(un.sun_path)) {
		errno = ENAMETOOLONG;
		return -1; // name too long
	}
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		return -2; // socket error
	}
	unlink(name);
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, name);
	len = offsetof(struct sockaddr_un, sun_path) + strlen(name);
	if (bind(fd, (struct sockaddr*)&un, len) < 0) {
		rval = -3; // bind error
		goto errout;
	}
	if (listen(fd, qlen) < 0) {
		rval = -4; // listen error
		goto errout;
	}
	return fd;
errout:
	err = errno;
	close(fd);
	errno = err;
	return rval;
}

int unix_socket_serv_accept(int listenfd, uid_t* uidptr) {
	int clifd, err, rval;
	struct sockaddr_un un;
	socklen_t len = sizeof(un);
	struct ucred credentials;
	socklen_t ucred_len = sizeof(struct ucred);
	if ((clifd = accept(listenfd, (struct sockaddr*)&un, &len)) < 0) {
		return -1; // accept error
	}
	if (getsockopt(clifd, SOL_SOCKET, SO_PEERCRED, &credentials, &ucred_len) < 0) {
		rval = -2; // getsockopt error
		goto errout;
	}
	if (uidptr != NULL) {
		*uidptr = credentials.uid;
	}
	return clifd;
errout:
	err = errno;
	close(clifd);
	errno = err;
	return rval;
}

int unix_socket_cli_conn(const char* name) {
	int fd, len, err, rval;
	struct sockaddr_un sun;
	memset(&sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;
	if (strlen(name) >= sizeof(sun.sun_path)) {
		errno = ENAMETOOLONG;
		return -1; // name too long
	}
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		return -2; // socket error
	}
	strcpy(sun.sun_path, name);
	len = offsetof(struct sockaddr_un, sun_path) + strlen(name);
	if (connect(fd, (struct sockaddr*)&sun, len) < 0) {
		rval = -3; // connect error
		goto errout;
	}
	return fd;
errout:
	err = errno;
	close(fd);
	errno = err;
	return rval;
}

int unix_socket_send_fd(int sockfd, int fd) {
	int domain;
	socklen_t len = sizeof(domain);
	if (getsockopt(sockfd, SOL_SOCKET, SO_DOMAIN, &domain, &len) == -1) {
		err_ret("call getsockopt");
		return -1;
	}
	if (domain != AF_UNIX) {
		err_ret("SCM_RIGHTS only works on AF_UNIX sockets");
		return -1;
	}
	struct msghdr msg = { 0 };
	char buf[1] = { 0 };
	struct iovec iov;
	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	union {
		char buf[CMSG_SPACE(sizeof(int))];
		struct cmsghdr align;
	} u;
	memset(&u, 0, sizeof(u));
	msg.msg_control = u.buf;
	msg.msg_controllen = sizeof(u.buf);

	struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
	if (!cmsg) {
		err_ret("CMSG_FIRSTHDR returned NULL");
		return -1;
	}
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	*reinterpret_cast<int*>(CMSG_DATA(cmsg)) = fd;

	if (sendmsg(sockfd, &msg, 0) < 0) {
		err_ret("call sendmsg");
		return -1;
	}
	return 0;
}

int unix_socket_recv_fd(int socket_fd, uid_t* uidptr) {
	int on = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) < 0) {
		err_ret("call setsockopt");
	}
	struct msghdr msg = { 0 };
	char buf[1];
	struct iovec iov;
	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	union {
		char buf[CMSG_SPACE(sizeof(int)) + CMSG_SPACE(sizeof(struct ucred))];
		struct cmsghdr align;
	} u;
	memset(&u, 0, sizeof(u));
	msg.msg_control = u.buf;
	msg.msg_controllen = sizeof(u.buf);
	if (recvmsg(socket_fd, &msg, 0) < 0) {
		err_ret("call recvmsg");
		return(-1);
	}
	int received_fd = -1;
	struct ucred creds = { 0, 0, 0 };
	for (struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg); cmsg != nullptr; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
			received_fd = *reinterpret_cast<int*>(CMSG_DATA(cmsg));
		}
		if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_CREDENTIALS) {
			creds = *reinterpret_cast<struct ucred*>(CMSG_DATA(cmsg));
			if (uidptr != NULL) {
				*uidptr = creds.uid;
			}
		}
	}
	if (received_fd == -1) {
		err_msg("no fd received via socket");
		return(-1);
	}
	return received_fd;
}

void unix_socket_server_loop(int sockfd) {
	struct io_uring ring;
	if (io_uring_queue_init(UNIX_SOCKET_QUEUE_DEPTH, &ring, 0) < 0) {
		err_sys("[Server] call io_uring_queue_init");
	}
	// get client ring fd
	int client_ring_fd = unix_socket_recv_fd(sockfd, NULL);
	if (client_ring_fd < 0)
		err_quit("[Server] failed to recv client ring");
	// register client ring fd at index 0
	int files[] = { client_ring_fd };
	if (io_uring_register_files(&ring, files, 1) < 0)
		err_sys("[Server] call io_uring_register_files");
	char buf[UNIX_SOCKET_MAX_MSG_SIZE];
	while (true) {
		// read request from socket
		int n = read(sockfd, buf, sizeof(buf));
		if (n <= 0)
			break; // Client closed
		std::stringstream ss(std::string(buf, n));
		std::string cmd, path_str;
		int mode = 0;
		int target_fd = -1;
		if ((ss >> cmd >> path_str >> mode) && (cmd == UNIX_SOCKET_CL_OPEN)) {
			target_fd = open(path_str.c_str(), mode);
		}
		struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
		if (target_fd < 0) {
			// send error back to client (data = -errno)
			io_uring_prep_msg_ring(sqe, 0, (errno > 0 ? -errno : -1), 0, 0);
		}
		else {
			// send file descriptor back to client at index 1
			io_uring_prep_msg_ring_fd(sqe, 0, target_fd, 1, 0, 0);
		}
		io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);
		io_uring_submit(&ring);
		struct io_uring_cqe* cqe;
		io_uring_wait_cqe(&ring, &cqe);
		if (cqe->res < 0)
			err_cont(-cqe->res, "[Server] Failed to send fd to ring");
		io_uring_cqe_seen(&ring, cqe);
		if (target_fd >= 0)
			close(target_fd);
	}
	io_uring_queue_exit(&ring);
}

int unix_socket_client_open(struct io_uring* ring, const char* name, mode_t oflag) {
	static int sockfd[2] = { -1, -1 };
	static bool ring_inited = false;
	if (!ring_inited) {
		if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) < 0)
			err_sys("call socketpair");
		pid_t pid = fork();
		if (pid < 0)
			err_sys("fork");
		if (pid == 0) { // Child (Server)
			close(sockfd[0]);
			unix_socket_server_loop(sockfd[1]);
			exit(0);
		}
		// Parent (Client)
		close(sockfd[1]);
		int files[2] = { -1, -1 };
		if (io_uring_register_files(ring, files, 2) < 0)
			err_sys("call io_uring_register_files");
		if (unix_socket_send_fd(sockfd[0], ring->ring_fd) < 0)
			return -1;
		ring_inited = true;
	}
	// send open request to server
	std::stringstream ss;
	ss << UNIX_SOCKET_CL_OPEN << " " << name << " " << oflag;
	std::string req = ss.str();
	if (write(sockfd[0], req.c_str(), req.length()) < 0)
		err_sys("call write");
	// wait for request from server
	struct io_uring_cqe* cqe;
	io_uring_wait_cqe(ring, &cqe);
	int res = cqe->res;
	io_uring_cqe_seen(ring, cqe);
	if (res < 0) {
		errno = -res;
		return -1;
	}
	// if res >= 0, return index received fd in fixed table
	return 1;
}