module;
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <climits>
#include <cerrno>
#include <cstring>
module apue;


/**
 @brief clear flags to fd
 @param fd File descriptor to modify.
 @param flags Flags to set.
 @throws UnixError on fcntl error.
*/
void clr_fl(int fd, int flags) {
	int val;
	if ((val = fcntl(fd, F_GETFL, 0)) < 0)
		err_ret("call fcntl with F_GETFL");
	val &= ~flags;
	if (fcntl(fd, F_SETFL, val) < 0)
		err_ret("call fcntl with F_SETFL");
}

/**
 @brief set flags to fd
 @param fd File descriptor to modify.
 @param flags Flags to set.
 @throws UnixError on fcntl error.
*/
void set_fl(int fd, int flags) {
	int val;
	if ((val = fcntl(fd, F_GETFL, 0)) < 0)
		err_ret("call fcntl with F_GETFL");
	val |= flags;
	if (fcntl(fd, F_SETFL, val) < 0)
		err_ret("call fcntl with F_SETFL");
}

/**
 * @brief Allocates a buffer for a path, using system limits.
 * @return std::vector<char> containing the buffer.
 * @throws UnixError on pathconf error.
 * @throws std::runtime_error on memory allocation error.
 */
std::vector<char> path_alloc() {
	static long pathmax = 0;
	static long posix_version = 0;
	static long xsi_version = 0;

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
			} else {
				pathmax++;
			}
		}
	}

	size_t size;
	if ((posix_version < 200112L) && (xsi_version < 4))
		size = pathmax + 1;
	else
		size = pathmax;

	try {
		std::vector<char> buffer(size);
		return buffer;
	} catch (const std::bad_alloc&) {
		throw std::runtime_error("malloc error for pathname");
	}
}


/**
 * @brief Returns the maximum number of files a process can have open.
 * Caches the result on the first call.
 */
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
			} else {
				err_ret("sysconf error for _SC_OPEN_MAX");
			}
		}
	}
	return openmax;
}
