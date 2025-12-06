#include "apue.h"

constexpr int QLEN = 10;

int serv_unix_socket_listen(const char* name) {
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
	if (listen(fd, QLEN) < 0) {
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

int serv_unix_socket_accept(int listenfd, uid_t* uidptr) {
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


int cli_unix_socket_conn(const char* name) {
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