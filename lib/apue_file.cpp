#include "apue.h"

ssize_t readn(int fd, void* ptr, size_t n) {
	size_t nleft;
	ssize_t nread;
	nleft = n;
	char* buf = static_cast<char*>(ptr);
	while (nleft > 0) {
		if ((nread = read(fd, buf, nleft)) < 0) {
			if (nleft == n)
				return(-1);
			else
				break;
		}
		else if (nread == 0) {
			break;
		}
		nleft -= nread;
		buf += nread;
	}
	return(n - nleft);
}

ssize_t writen(int fd, void* ptr, size_t n) {
	size_t nleft;
	ssize_t nwritten;
	nleft = n;
	char* buf = static_cast<char*>(ptr);
	while (nleft > 0) {
		if ((nwritten = write(fd, buf, nleft)) < 0) {
			if (nleft == n)
				return(-1);
			else
				break;
		}
		else if (nwritten == 0) {
			break;
		}
		nleft -= nwritten;
		buf += nwritten;
	}
	return(n - nleft);
}

int set_cloexec(int fd) {
	int val;
	if ((val = fcntl(fd, F_GETFD, 0)) < 0)
		return(-1);
	val |= FD_CLOEXEC;
	return(fcntl(fd, F_SETFD, val));
}

void clr_fl(int fd, int flags) {
	int val;
	if ((val = fcntl(fd, F_GETFL, 0)) < 0)
		err_ret("call fcntl with F_GETFL");
	val &= ~flags;
	if (fcntl(fd, F_SETFL, val) < 0)
		err_ret("call fcntl with F_SETFL");
}

void set_fl(int fd, int flags) {
	int val;
	if ((val = fcntl(fd, F_GETFL, 0)) < 0)
		err_ret("call fcntl with F_GETFL");
	val |= flags;
	if (fcntl(fd, F_SETFL, val) < 0)
		err_ret("call fcntl with F_SETFL");
}

std::string path_alloc() {
	static std::size_t pathmax = 0;
	static std::size_t posix_version = 0;
	static std::size_t xsi_version = 0;
	if (posix_version == 0)
		posix_version = sysconf(_SC_VERSION);
	if (xsi_version == 0)
		xsi_version = sysconf(_SC_XOPEN_VERSION);
	if (pathmax == 0) {
#ifdef PATH_MAX
		pathmax = PATH_MAX;
#else
		pathmax = 0;
#endif
		if (pathmax == 0) {
			errno = 0;
			if ((pathmax = pathconf("/", _PC_PATH_MAX)) < 0) {
				if (errno == 0)
					pathmax = PATH_MAX_GUESS;
				else
					err_ret("pathconf error for _PC_PATH_MAX");
			}
			else {
				pathmax++;
			}
		}
	}

	try {
		std::string buffer;
		buffer.resize_and_overwrite(pathmax, [](char* buf, std::size_t count) { return count; });
		return buffer;
	}
	catch (const std::bad_alloc&) {
		throw std::runtime_error("malloc error for path_alloc");
	}
}

long open_max() {
	static long openmax = 0;
#ifdef OPEN_MAX
	openmax = OPEN_MAX;
#endif
	if (openmax == 0) {
		errno = 0;
		if ((openmax = sysconf(_SC_OPEN_MAX)) < 0) {
			if (errno == 0) {
				openmax = OPEN_MAX_GUESS;
			}
			else {
				err_ret("sysconf error for _SC_OPEN_MAX");
			}
		}
	}
	return openmax;
}