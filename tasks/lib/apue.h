#include "intrin.h"
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <print>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>



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

constexpr mode_t FILE_MODE = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
constexpr std::size_t PATH_MAX_GUESS = 1024;
constexpr std::size_t OPEN_MAX_GUESS = 256;
constexpr std::size_t MAXLINE = 4096;

std::string path_alloc();

long open_max();

void set_fl(int fd, int flags);

void clr_fl(int fd, int flags);

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
unsigned long long rdtsc() {
	return __rdtsc();
}