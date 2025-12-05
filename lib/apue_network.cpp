#include "apue.h"

int connect_ipv4_host(const char* host, int port, int type) {
	struct addrinfo hints, * ailist, * aip;
	int sockfd, err;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = type;
	hints.ai_family = AF_INET;
	std::string port_str = std::to_string(port);
	if ((err = getaddrinfo(host, port_str.c_str(), &hints, &ailist)) != 0) {
		err_ret("getaddrinfo error: {}", gai_strerror(err));
		return err;
	}
	for (aip = ailist; aip != NULL; aip = aip->ai_next) {
		if ((sockfd = connect_ipv4_addr((struct sockaddr_in*)aip->ai_addr, 0, type)) >= 0) {
			freeaddrinfo(ailist);
			return sockfd;
		}
	}
	freeaddrinfo(ailist);
	return -1;
}

int connect_ipv4_addr(const struct sockaddr_in* addr, int port, int type) {
	int sockfd = setup_socket_ipv4(port, type);
	if (connect(sockfd, (struct sockaddr*)addr, sizeof(*addr)) < 0) {
		close(sockfd);
		return -1;
	}
	return sockfd;
}

int setup_socket_ipv4(int port, int type) {
	int sockfd;
	struct sockaddr_in serv_addr;
	if ((sockfd = socket(AF_INET, type, 0)) < 0) {
		return -1;
	}
	int opt = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		close(sockfd);
		return -1;
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
		close(sockfd);
		return -1;
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; // IPv4
	serv_addr.sin_port = (port > 0) ? htons(port) : 0;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		close(sockfd);
		return -1;
	}
	return sockfd;
}

constexpr int QLEN = 10;

int serv_listen(const char* name) {
	int fd, len, err, rval;
	struct sockaddr_un un;
	if (strlen(name) >= sizeof(un.sun_path)) {
		errno = ENAMETOOLONG;
		return(-1);
	}
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		return(-2);
	unlink(name);
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, name);
	len = offsetof(struct sockaddr_un, sun_path) + strlen(name);
	if (bind(fd, (struct sockaddr*)&un, len) < 0) {
		rval = -3;
		goto errout;
	}
	if (listen(fd, QLEN) < 0) {
		rval = -4;
		goto errout;
	}
	return(fd);
errout:
	err = errno;
	close(fd);
	errno = err;
	return(rval);
}

constexpr int STALE = 30; // time after which we consider socket stale from

int serv_accept(int listenfd, uid_t* uidptr) {
	int clifd, err, rval;
	socklen_t len;
	time_t staletime;
	struct sockaddr_un un;
	struct stat statbuf;
	char* name;
	if ((name = malloc(sizeof(un.sun_path) + 1)) == NULL)
		return(-1);
	len = sizeof(un);
	if ((clifd = accept(listenfd, (struct sockaddr*)&un, &len)) < 0) {
		free(name);
		return(-2);
	}
	len -= offsetof(struct sockaddr_un, sun_path);
	memcpy(name, un.sun_path, len);
	name[len] = 0;
	if (stat(name, &statbuf) < 0) {
		rval = -3;
		goto errout;
	}
#ifdef S_ISSOCK
	if (S_ISSOCK(statbuf.st_mode) == 0) {
		rval = -4; // not socket
		goto errout;
	}
#endif
	if ((statbuf.st_mode & (S_IRWXG | S_IRWXO)) || (statbuf.st_mode & S_IRWXU) != S_IRWXU) {
		rval = -5; // not rwx------ 
		goto errout;
	}
	staletime = time(NULL) - STALE;
	if (statbuf.st_atime < staletime || statbuf.st_ctime < staletime || statbuf.st_mtime < staletime) {
		rval = -6; // inode is stale
		goto errout;
	}
	if (uidptr != NULL)
		*uidptr = statbuf.st_uid; /* вернуть UID клиента */
	unlink(un.sun_path); /* работа с файлом закончена */
	free(name);
	return(clifd);
errout:
	err = errno;
	close(clifd);
	free(name);
	errno = err;
	return(rval);
}