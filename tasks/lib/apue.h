#ifndef APUE_H
#define APUE_H

#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <print>
#include <signal.h>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>


constexpr mode_t FILE_MODE = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
constexpr std::size_t PATH_MAX_GUESS = 1024;
constexpr std::size_t OPEN_MAX_GUESS = 256;
constexpr std::size_t MAXLINE = 4096;


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
 @brief set flags to fd
 @param fd File descriptor to modify.
 @param flags Flags to set.
 @throws UnixError on fcntl error.
*/
void set_fl(int fd, int flags);

/**
 @brief clear flags to fd
 @param fd File descriptor to modify.
 @param flags Flags to set.
 @throws UnixError on fcntl error.
*/
void clr_fl(int fd, int flags);

/**
 @brief Print exit status of a child process.
 @param status Status value returned by wait or waitpid.
*/
void pr_exit(int status);

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

void TELL_WAIT(void);
void TELL_PARENT(pid_t);
void TELL_CHILD(pid_t);
void WAIT_PARENT(void);
void WAIT_CHILD(void);

#endif // !APUE_H
