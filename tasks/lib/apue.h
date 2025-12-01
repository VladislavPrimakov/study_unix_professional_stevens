#ifndef APUE_H
#define APUE_H

#include <cerrno>
#include <chrono>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <optional>
#include <print>
#include <signal.h>
#include <string>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <syslog.h>
#include <unistd.h>
#include <vector>

constexpr const char* LOCKFILE = "/var/run/daemon.pid";
constexpr mode_t LOCKMODE(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
constexpr mode_t FILE_MODE = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
constexpr std::size_t PATH_MAX_GUESS = 1024;
constexpr std::size_t OPEN_MAX_GUESS = 256;
constexpr std::size_t MAXLINE = 4096;

using Sigfunc = void(int);
using ThreadFunc = void* (*)(void*);

template<typename... Args>
std::string format_message(bool addErrno, const std::string& fmt, Args&&... args) {
	try {
		std::string msg = "Error: " + std::vformat(fmt, std::make_format_args(args...));
		if (addErrno) {
			msg += ": " + std::string(strerror(errno));
		}
		return msg;
	}
	catch (const std::format_error& e) {
		return "Formatting error: " + fmt + " (" + e.what() + ")";
	}
}

template<typename... Args>
std::string format_message(int err_code, const std::string& fmt, Args&&... args) {
	std::string msg = format_message(false, fmt, std::forward<Args>(args)...);
	msg += ": " + std::string(strerror(err_code));
	return msg;
}

/**
 * @brief Reads exactly nbytes from a file descriptor into a buffer.
 * @param fd File descriptor to read from.
 * @param ptr Buffer to read into.
 * @param n Number of bytes to read.
 * @return Number of bytes read, or -1 on error.
 */
ssize_t readn(int fd, void* ptr, size_t n);

/**
 * @brief Writes exactly nbytes from a buffer to a file descriptor.
 * @param fd File descriptor to write to.
 * @param ptr Buffer to write from.
 * @param n Number of bytes to write.
 * @return Number of bytes written, or -1 on error.
 */
ssize_t writen(int fd, void* ptr, size_t n);

/**
 * @brief Converts a std::chrono::system_clock::time_point to a timespec structure.
 * @param tp Time point to convert.
 * @return Shared pointer to a timespec structure.
 */
struct timespec to_timespec(const std::chrono::system_clock::time_point& tp);

/**
 * @brief Converts a std::chrono::nanoseconds duration to a timespec structure.
 * @param ns Duration in nanoseconds to convert.
 * @return Shared pointer to a timespec structure.
 */
struct timespec to_timespec(const std::chrono::nanoseconds& ns);

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

/**
 @brief Print user message + given errno. Exit(1).
*/
template<typename... Args>
void err_exit(int err, const std::string& fmt, Args&&... args) {
	std::println(std::cerr, "{}", format_message(err, fmt, std::forward<Args>(args)...));
	std::exit(1);
}

/**
 @brief Print user message + given errno.
*/
template<typename... Args>
void err_cont(int err, const std::string& fmt, Args&&... args) {
	std::println(std::cerr, "{}", format_message(err, fmt, std::forward<Args>(args)...));
}

/**
 @brief Print user message + errno. Exit(1).
*/
template<typename... Args>
void err_sys(const std::string& fmt, Args&&... args) {
	std::println(std::cerr, "{}", format_message(true, fmt, std::forward<Args>(args)...));
	std::exit(1);
}

/**
 @brief Print user message + errno. Abort().
*/
template<typename... Args>
void err_dump(const std::string& fmt, Args&&... args) {
	std::println(std::cerr, "{}", format_message(true, fmt, std::forward<Args>(args)...));
	std::abort();
}

/**
 @brief Print user message + errno.
*/
template<typename... Args>
void err_ret(const std::string& fmt, Args&&... args) {
	std::println(std::cerr, "{}", format_message(true, fmt, std::forward<Args>(args)...));
}

/**
@brief Print user message. Exit(1).
*/
template<typename... Args>
void err_quit(const std::string& fmt, Args&&... args) {
	std::println(std::cerr, "{}", format_message(false, fmt, std::forward<Args>(args)...));
	std::exit(1);
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

#endif // !APUE_H
