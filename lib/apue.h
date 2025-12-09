#ifndef APUE_H
#define APUE_H

#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <liburing.h>
#include <memory>
#include <netdb.h>
#include <optional>
#include <print>
#include <signal.h>
#include <sstream>
#include <string>
#include <string.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <syslog.h>
#include <unistd.h>
#include <vector>

constexpr const char* LOCKFILE = "/var/run/daemon.pid";
constexpr mode_t LOCKMODE = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
constexpr mode_t FILE_MODE = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
constexpr std::size_t PATH_MAX_GUESS = 1024;
constexpr std::size_t OPEN_MAX_GUESS = 256;
constexpr std::size_t MAXLINE = 4096;

// commands via unix domain socket
enum class UNIX_SOCKET_COMMAND {
	OPEN,
};

// default unix domain socket path for daemon communication
constexpr char UNIX_SOCKET_CS_OPEN[] = "/tmp/daemon.socket";

// maximum message size for unix domain socket communication
constexpr std::size_t UNIX_SOCKET_MAX_MSG_SIZE = 1024;

// maximum error message size for unix domain socket communication
constexpr std::size_t UNIX_SOCKET_MAX_ERR_MSG_SIZE = 1024;

using Sigfunc = void(int);
using ThreadFunc = void* (*)(void*);

struct StatusMsg {
	int code;
	std::string msg;
	StatusMsg() : code(0), msg("") {}
	StatusMsg(int c, const std::string& m) : code(c), msg(m) {}
	bool hasError() const {
		return code != 0 || !msg.empty();
	}
	std::string toString() const {
		std::stringstream ss;
		if (!msg.empty()) ss << msg;
		if (code != 0) {
			if (!msg.empty()) ss << ": ";
			ss << strerror(code);
		}
		return ss.str();
	}
};

// flag to log messages to stderr or syslog
inline bool log_to_stderr = true;

/**
 @brief Format error message with optional errno description.
 @param err_code Error code (errno). If negative, no errno description is appended.
 @param fmt Format string.
 @param args Arguments for format string.
 @return Formatted error message string.
*/
template<typename... Args>
std::string format_message(int err_code, const std::string& fmt, Args&&... args) {
	try {
		std::string msg = std::vformat(fmt, std::make_format_args(args...));
		if (log_to_stderr) {
			msg = "Error: " + msg;
		}
		if (err_code >= 0) {
			msg += ": " + std::string(strerror(err_code));
		}
		return msg;
	}
	catch (const std::format_error& e) {
		return "Formatting error: " + fmt + " (" + e.what() + ")";
	}
}

inline void do_error(const std::string& msg) {
	if (log_to_stderr) {
		std::println(std::cerr, "{}", msg);
	}
	else {
		syslog(LOG_ERR, "%s", msg.c_str());
	}
}

/**
 @brief Print user message + given errno. Exit(1).
*/
template<typename... Args>
void err_exit(int err, const std::string& fmt, Args&&... args) {
	do_error(format_message(err, fmt, std::forward<Args>(args)...));
	std::exit(1);
}

/**
 @brief Print user message + given errno.
*/
template<typename... Args>
void err_cont(int err, const std::string& fmt, Args&&... args) {
	do_error(format_message(err, fmt, std::forward<Args>(args)...));
}

/**
 @brief Print user message + errno. Exit(1).
*/
template<typename... Args>
void err_sys(const std::string& fmt, Args&&... args) {
	do_error(format_message(errno, fmt, std::forward<Args>(args)...));
	std::exit(1);
}

/**
 @brief Print user message + errno. Abort().
*/
template<typename... Args>
void err_dump(const std::string& fmt, Args&&... args) {
	do_error(format_message(errno, fmt, std::forward<Args>(args)...));
	std::abort();
}

/**
 @brief Print user message + errno.
*/
template<typename... Args>
void err_ret(const std::string& fmt, Args&&... args) {
	do_error(format_message(errno, fmt, std::forward<Args>(args)...));
}

/**
@brief Print user message. Exit(1).
*/
template<typename... Args>
void err_quit(const std::string& fmt, Args&&... args) {
	do_error(format_message(-1, fmt, std::forward<Args>(args)...));
	std::exit(1);
}

/**
 @brief Print user message.
*/
template<typename... Args>
void err_msg(const std::string& fmt, Args&&... args) {
	do_error(format_message(-1, fmt, std::forward<Args>(args)...));
}

/**
 @brief Read the time-stamp counter.
*/
inline uint64_t rdtsc() {
	unsigned int lo, hi, aux;
	__asm__ __volatile__("rdtscp" : "=a"(lo), "=d"(hi), "=c"(aux));
	return ((uint64_t)hi << 32) | lo;
}

void TELL_WAIT_SIGNAL();
void TELL_DONE_SIGNAL();
void TELL_PARENT_SIGNAL(pid_t);
void TELL_CHILD_SIGNAL(pid_t);
void WAIT_PARENT_SIGNAL();
void WAIT_CHILD_SIGNAL();

void TELL_WAIT_PIPE();
void TELL_DONE_PIPE();
void TELL_PARENT_PIPE();
void TELL_CHILD_PIPE();
void WAIT_PARENT_PIPE();
void WAIT_CHILD_PIPE();

/**
@brief Print the signal mask of the calling process.
@param str Message to print before the mask.
*/
void pr_mask(const std::string& str);

/**
 @brief Simplified signal handling function.
 @param signo Signal number.
 @param func Pointer to the signal handling function.
 @return Previous signal handling function pointer.
*/
Sigfunc* apue_signal(int signo, Sigfunc* func);

/**
 * @brief Connects to an IPv4 host by name.
 * @param host Hostname or IP address as a string.
 * @param port Port number to connect to. If <= 0, set free port.
 * @param type Socket type (e.g., SOCK_STREAM).
 * @return File descriptor of the connected socket, or -1 on failure.
 */
int connect_ipv4_host(const char* host, int port, int type);

/**
 * @brief Connects to an IPv4 by address.
 * @param addr Pointer to the sockaddr_in structure.
 * @param port Port number to connect to. If <= 0, set free port.
 * @param type Socket type (e.g., SOCK_STREAM).
 * @return File descriptor of the connected socket, or -1 on failure.
 */
int connect_ipv4_addr(const struct sockaddr_in* addr, int port, int type);

/**
 * @brief Sets up a server socket for IPv4.
 * @param port Port number to bind to. If <= 0, set free port.
 * @param type Socket type (e.g., SOCK_STREAM).
 * @return File descriptor of the listening socket, or -1 on failure.
 */
int setup_socket_ipv4(int port, int type);


/**
 * @brief Sets up a UNIX domain socket server and listens for connections.
 * @param name Pathname for the UNIX domain socket.
 * @param qlen Queue length for pending connections.
 * @return File descriptor of the listening socket, or negative error code on failure.
 */
int unix_socket_serv_listen(const char* name, unsigned int qlen);

/**
 * @brief Accepts a connection on a UNIX domain socket server.
 * @param listenfd File descriptor of the listening socket.
 * @param uidptr Pointer to store the UID of the connecting client (optional).
 * @return File descriptor of the connected socket, or negative error code on failure.
 */
int unix_socket_serv_accept(int listenfd, uid_t* uidptr);

/**
 * @brief Connects to a UNIX domain socket server.
 * @param name Pathname of the UNIX domain socket to connect to.
 * @return File descriptor of the connected socket, or negative error code on failure.
 */
int unix_socket_cli_conn(const char* name);

/**
 * @brief Receives a file descriptor over a UNIX domain socket.
 * @param socket_fd File descriptor of the UNIX domain socket.
 * @param uidptr Pointer to store the UID of the sending process (optional).
 * @return Received file descriptor, or -1 on failure.
 */
int unix_socket_recv_fd(int socket_fd, uid_t* uidptr);

/**
 * @brief Sends a file descriptor over a UNIX domain socket.
 * @param socket_fd File descriptor of the UNIX domain socket.
 * @param fd_to_send File descriptor to send.
 * @param status Optional pointer to StatusMsg to receive status information.
 * @return 0 on success, -1 on failure.
 */
int unix_socket_send_fd(int socket_fd, int fd_to_send, struct StatusMsg* status = nullptr);

/**
 * @brief Processes a request received over a UNIX domain socket.
 * @param client_fd File descriptor of the connected client socket.
 * @param request_str Request string received from the client.
 */
void unix_socket_process_request(int client_fd, const std::string& request_str);

/**
 * @brief Sends a request to a UNIX domain socket server.
 * @param sockfd File descriptor of the connected UNIX domain socket.
 * @param cmd Command to send.
 * @param path Path argument for the command.
 * @param mode Mode argument for the command (if applicable).
 * @return 0 on success, -1 on failure.
 */
int unix_socket_send_request(int sockfd, UNIX_SOCKET_COMMAND cmd, const std::string& path, int mode);

/**
 * @brief Checks if the given socket is a UNIX domain socket.
 * @param socket_fd File descriptor of the socket to check.
 * @return 0 if it is a UNIX domain socket, -1 otherwise.
 */
int unix_socket_check_domain(int socket_fd);

/**
 * @brief Client function to request opening a file via UNIX domain socket.
 * @param name Pathname of the UNIX domain socket server.
 * @param oflag Flags for opening the file (e.g., O_RDONLY).
 * @return File descriptor of the opened file, or negative error code on failure.
 */
int unix_socket_client_open(const char* name, mode_t oflag);

/**
 * @brief Reads exactly nbytes from a file descriptor into a buffer.
 * @param fd File descriptor to read from.
 * @param ptr Buffer to read into.
 * @param n Number of bytes to read.
 * @return Optional containing number of bytes read, or std::nullopt on error.
 */
std::optional<size_t> readn(int fd, void* ptr, size_t n);

/**
 * @brief Writes exactly nbytes from a buffer to a file descriptor.
 * @param fd File descriptor to write to.
 * @param ptr Buffer to write from.
 * @param n Number of bytes to write.
 * @return Optional containing number of bytes written, or std::nullopt on error.
 */
std::optional<size_t> writen(int fd, void* ptr, size_t n);

/**
 * @brief Converts a std::chrono::system_clock::time_point to a timespec structure.
 * @param tp Time point to convert.
 * @return Shared pointer to a timespec structure.
 */
struct timespec to_timespec(const std::chrono::system_clock::time_point& tp);


template <typename Rep, typename Period>
struct timespec to_timespec(const std::chrono::duration<Rep, Period>& d) {
	using namespace std::chrono;
	auto total_ns = duration_cast<nanoseconds>(d);
	auto secs = duration_cast<seconds>(total_ns);
	auto rem_nanos = total_ns - secs;
	return {
		static_cast<time_t>(secs.count()),
		static_cast<long>(rem_nanos.count())
	};
}

/**
 * @brief Converts a timespec structure to a std::chrono::system_clock::time_point.
 * @param ts Timespec structure to convert.
 * @return Corresponding time_point.
 */
std::chrono::system_clock::time_point to_time_point(const struct timespec& ts);

/**
 * @brief Creates a detached thread.
 * @param fn Thread function to execute.
 * @param arg Argument to pass to the thread function.
 * @return 0 on success, error number on failure.
 */
int makethread(ThreadFunc fn, void* arg);

/**
 * @brief Sets the close-on-exec (FD_CLOEXEC) flag on the given file descriptor.
 * @param fd File descriptor to modify.
 * @return 0 on success, -1 on failure.
 */
int set_cloexec(int fd);

/**
 * @brief Applies a write lock on the entire file associated with the given file descriptor.
 * @param fd File descriptor of the file to lock.
 * @return 0 on success, -1 on failure.
 */
int lockfile(int fd);

/**
 * @brief Daemonizes the calling process.
 * @param cmd Command name for syslog.
 */
void daemonize(const char* cmd);

/**
 * @brief Checks if the daemon is already running using a lock file.
 * @return 1 if already running, 0 otherwise.
 */
int already_running(void);

/**
 * @brief Executes a command string by invoking the system shell.
 * @param cmdstring Command string to execute.
 * @return Exit status of the command, or -1 on error.
 */
int system(const char* cmdstring);

/**
 * @brief Allocates a buffer for a path, using system limits.
 * @return std::string with size PATH_MAX.
 * @throws std::runtime_error on memory allocation error.
 */
std::string path_alloc();

/**
 * @brief Returns the maximum number of files a process can have open.
 * Caches the result on the first call.
 */
long open_max();

/**
 @brief Apply, remove, or test a record lock on a file.
 @param fd File descriptor of the file to lock.
 @param cmd F_SETLK, F_SETLKW, or F_GETLK.
 @param type F_RDLCK, F_WRLCK, or F_UNLCK.
 @param offset Byte offset where the lock begins.
 @param whence SEEK_SET, SEEK_CUR, or SEEK_END.
 @param len Number of bytes to lock; 0 means to EOF.
 @return 0 on success, -1 on failure.
*/
int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len);

template <int Cmd, int Type>
inline int file_lock(int fd, off_t offset, int whence, off_t len) {
	return lock_reg(fd, Cmd, Type, offset, whence, len);
}

inline int read_lock(int fd, off_t offset, int whence, off_t len) {
	return file_lock<F_SETLK, F_RDLCK>(fd, offset, whence, len);
}

inline int readw_lock(int fd, off_t offset, int whence, off_t len) {
	return file_lock<F_SETLKW, F_RDLCK>(fd, offset, whence, len);
}

inline int write_lock(int fd, off_t offset, int whence, off_t len) {
	return file_lock<F_SETLK, F_WRLCK>(fd, offset, whence, len);
}

inline int writew_lock(int fd, off_t offset, int whence, off_t len) {
	return file_lock<F_SETLKW, F_WRLCK>(fd, offset, whence, len);
}

inline int un_lock(int fd, off_t offset, int whence, off_t len) {
	return file_lock<F_SETLK, F_UNLCK>(fd, offset, whence, len);
}

/**
 @brief Tests for a record lock on a file.
 @param fd File descriptor of the file to test.
 @param type F_RDLCK or F_WRLCK.
 @param offset Byte offset where the lock begins.
 @param whence SEEK_SET, SEEK_CUR, or SEEK_END.
 @param len Number of bytes to test; 0 means to EOF.
 @return 0 if the lock is not held by another process; otherwise, returns the PID of the process holding the lock.
*/
pid_t lock_test(int fd, int type, off_t offset, int whence, off_t len);

template <int Type>
inline bool is_lockable(int fd, off_t offset, int whence, off_t len) {
	return lock_test(fd, Type, offset, whence, len) == 0;
}

/**
 @brief Checks if a read lock can be applied to the specified region of a file.
 @param fd File descriptor of the file to test.
 @param offset Byte offset where the lock begins.
 @param whence SEEK_SET, SEEK_CUR, or SEEK_END.
 @param len Number of bytes to test; 0 means to EOF.
 @return true if the read lock can be applied; false otherwise.
*/
inline bool is_read_lockable(int fd, off_t offset, int whence, off_t len) {
	return is_lockable<F_RDLCK>(fd, offset, whence, len);
}

/**
 @brief Checks if a write lock can be applied to the specified region of a file.
 @param fd File descriptor of the file to test.
 @param offset Byte offset where the lock begins.
 @param whence SEEK_SET, SEEK_CUR, or SEEK_END.
 @param len Number of bytes to test; 0 means to EOF.
 @return true if the write lock can be applied; false otherwise.
*/
inline bool is_write_lockable(int fd, off_t offset, int whence, off_t len) {
	return is_lockable<F_WRLCK>(fd, offset, whence, len);
}

/**
 @brief set flags to fd
 @param fd File descriptor to modify.
 @param flags Flags to set.
*/
void set_fl(int fd, int flags);

/**
 @brief clear flags to fd
 @param fd File descriptor to modify.
 @param flags Flags to set.
*/
void clr_fl(int fd, int flags);

/**
 @brief Print exit status of a child process.
 @param status Status value returned by wait or waitpid.
*/
void pr_exit(int status);

#endif // !APUE_H
