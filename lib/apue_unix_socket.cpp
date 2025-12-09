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




struct WireHeader {
	int code;
	int msg_len;
};

int unix_socket_check_domain(int socket_fd) {
	int domain;
	socklen_t len = sizeof(domain);
	if (getsockopt(socket_fd, SOL_SOCKET, SO_DOMAIN, &domain, &len) == -1) {
		err_ret("[unix_socket_check_domain] call getsockopt");
		return -1;
	}
	if (domain != AF_UNIX) {
		return -1;
	}
	return 0;
}

int unix_socket_send_fd(int socket_fd, int fd_to_send, struct StatusMsg* status) {
	if (unix_socket_check_domain(socket_fd) < 0) {
		return -1;
	}
	struct msghdr msg = { 0 };
	StatusMsg default_status;
	StatusMsg* p_status = (status != nullptr) ? status : &default_status;
	WireHeader header;
	header.code = p_status->code;
	header.msg_len = static_cast<int>(std::min(p_status->msg.size(), UNIX_SOCKET_MAX_ERR_MSG_SIZE));
	struct iovec iov[2];
	iov[0].iov_base = &header;
	iov[0].iov_len = sizeof(WireHeader);
	iov[1].iov_base = const_cast<char*>(p_status->msg.data());
	iov[1].iov_len = header.msg_len;
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;
	union {
		char buf[CMSG_SPACE(sizeof(int))];
		struct cmsghdr align;
	} u;
	if (fd_to_send >= 0 && !p_status->hasError()) {
		std::memset(&u, 0, sizeof(u));
		msg.msg_control = u.buf;
		msg.msg_controllen = sizeof(u.buf);
		struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;
		cmsg->cmsg_len = CMSG_LEN(sizeof(int));
		*reinterpret_cast<int*>(CMSG_DATA(cmsg)) = fd_to_send;
	}
	else {
		msg.msg_control = nullptr;
		msg.msg_controllen = 0;
	}
	if (sendmsg(socket_fd, &msg, 0) < 0) {
		err_ret("[unix_socket_send_fd] call sendmsg");
		return -1;
	}
	return 0;
}

int unix_socket_recv_fd(int socket_fd, uid_t* uidptr) {
	if (unix_socket_check_domain(socket_fd) < 0) {
		return -1;
	}
	int on = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) < 0) {
		err_ret("[unix_socket_recv_fd] call setsockopt(SO_PASSCRED)");
	}
	struct msghdr msg = { 0 };
	WireHeader header;
	char text_buf[UNIX_SOCKET_MAX_ERR_MSG_SIZE];
	struct iovec iov[2];
	iov[0].iov_base = &header;
	iov[0].iov_len = sizeof(WireHeader);
	iov[1].iov_base = text_buf;
	iov[1].iov_len = sizeof(text_buf);
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;
	union {
		char buf[CMSG_SPACE(sizeof(int)) + CMSG_SPACE(sizeof(struct ucred))];
		struct cmsghdr align;
	} u;
	memset(&u, 0, sizeof(u));
	msg.msg_control = u.buf;
	msg.msg_controllen = sizeof(u.buf);
	ssize_t n = recvmsg(socket_fd, &msg, 0);
	if (n < 0) {
		err_ret("[unix_socket_recv_fd] call recvmsg");
		return -1;
	}
	if (static_cast<size_t>(n) < sizeof(WireHeader)) {
		err_msg("[unix_socket_recv_fd] Protocol error: message too short");
		return -1;
	}
	size_t actual_text_len = n - sizeof(WireHeader);
	if (header.msg_len != 0) {
		size_t safe_len = std::min(static_cast<size_t>(header.msg_len), actual_text_len);
		std::string remote_msg(text_buf, safe_len);
		auto status = StatusMsg(header.code, remote_msg);
		if (!status.hasError()) {
			err_msg(status.toString());
			return -1;
		}
	}
	int received_fd = -1;
	for (struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg); cmsg != nullptr; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
			received_fd = *reinterpret_cast<int*>(CMSG_DATA(cmsg));
		}
		if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_CREDENTIALS) {
			struct ucred creds = *reinterpret_cast<struct ucred*>(CMSG_DATA(cmsg));
			if (uidptr != NULL) {
				*uidptr = creds.uid;
			}
		}
	}
	if (received_fd == -1) {
		err_ret("[unix_socket_recv_fd] Protocol OK but no FD received");
	}
	return received_fd;
}

void unix_socket_server_loop(int socket_fd) {
	if (unix_socket_check_domain(socket_fd) < 0) {
		return;
	}
	char buf[UNIX_SOCKET_MAX_MSG_SIZE];
	while (true) {
		StatusMsg status;
		// read request from socket
		int n = read(socket_fd, buf, sizeof(buf));
		if (n == 0) {
			break; // Client closed
		}
		if (n < 0) {
			err_ret("[Server] read {}", socket_fd);
			break;
		}
		buf[n] = 0;
		std::stringstream ss(std::string(buf, n));
		std::string cmd, path_str;
		mode_t mode = 0;
		int target_fd = -1;
		if ((ss >> cmd >> path_str >> mode) && (cmd == UNIX_SOCKET_CL_OPEN)) {
			target_fd = open(path_str.c_str(), mode);
			if (target_fd < 0) {
				status.code = errno;
			}
		}
		else {
			status.msg = "Invalid request format or command";
		}
		if (status.hasError()) {
			unix_socket_send_fd(socket_fd, -1, &status);
		}
		else {
			unix_socket_send_fd(socket_fd, target_fd, nullptr);
			close(target_fd);
		}
	}
}

int unix_socket_client_open(const char* name, mode_t oflag) {
	int sockfd[2] = { -1, -1 };
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) < 0)
		err_sys("[Client] call socketpair");
	pid_t pid = fork();
	if (pid < 0) {
		close(sockfd[0]); close(sockfd[1]);
		err_sys("call fork");
	}
	if (pid == 0) { // Child (Server): read sockfd[1] for requests
		close(sockfd[0]);
		unix_socket_server_loop(sockfd[1]);
		close(sockfd[1]);
		exit(0);
	}
	// Parent (Client): send requests to sockdf[0] and wait for msg with fd 
	close(sockfd[1]);
	std::stringstream ss;
	ss << UNIX_SOCKET_CL_OPEN << " " << name << " " << oflag;
	std::string req = ss.str();
	if (!writen(sockfd[0], req.data(), req.length())) {
		err_sys("[Client] call write");
		return -1;
	}
	int fd = unix_socket_recv_fd(sockfd[0], NULL);
	close(sockfd[0]);
	waitpid(pid, NULL, 0);
	return fd;
}