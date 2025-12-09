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
	size_t msg_len;
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
	struct iovec iov;
	iov.iov_base = &header;
	iov.iov_len = sizeof(WireHeader);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
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
		err_msg("[unix_socket_recv_fd] Protocol message too short");
		return -1;
	}
	std::string remote_msg;
	if (header.msg_len > 0) {
		if (header.msg_len > UNIX_SOCKET_MAX_ERR_MSG_SIZE) {
			err_msg("[unix_socket_recv_fd] Protocol message error too long");
			return -1;
		}
		remote_msg.resize(header.msg_len);
		if (!readn(socket_fd, remote_msg.data(), header.msg_len)) {
			err_msg("[unix_socket_recv_fd] Protocol failed to read message body");
			return -1;
		}
	}
	StatusMsg status(header.code, remote_msg);
	if (status.hasError()) {
		err_msg(status.toString());
		return -1;
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

void unix_socket_process_request(int client_fd, const std::string& request_str) {
	std::stringstream ss(request_str);
	int op_int;
	StatusMsg status;
	int target_fd = -1;
	if (!(ss >> op_int)) {
		status.msg = "Empty request or invalid opcode format";
		unix_socket_send_fd(client_fd, -1, &status);
		return;
	}
	UNIX_SOCKET_COMMAND cmd = static_cast<UNIX_SOCKET_COMMAND>(op_int);
	switch (cmd) {
	case UNIX_SOCKET_COMMAND::OPEN: {
		std::string path_str;
		int mode_val = 0;
		if (ss >> path_str >> mode_val) {
			target_fd = open(path_str.c_str(), static_cast<mode_t>(mode_val));
			if (target_fd < 0) {
				status.code = errno;
			}
		}
		else {
			status.msg = "Invalid arguments for OPEN (expected: path mode)";
		}
		break;
	}
	default:
		status.msg = "Unknown command opcode: " + std::to_string(op_int);
		break;
	}
	if (status.hasError()) {
		unix_socket_send_fd(client_fd, -1, &status);
	}
	else {
		unix_socket_send_fd(client_fd, target_fd, nullptr);
	}
	if (target_fd >= 0) {
		close(target_fd);
	}
}

int unix_socket_send_request(int sockfd, UNIX_SOCKET_COMMAND cmd, const std::string& path, int mode) {
	std::stringstream ss;
	ss << static_cast<int>(cmd) << " " << path << " " << mode;
	std::string req = ss.str();
	if (!writen(sockfd, req.data(), req.length())) {
		err_ret("[unix_socket_send_request] call write");
		return -1;
	}
	return 0;
}