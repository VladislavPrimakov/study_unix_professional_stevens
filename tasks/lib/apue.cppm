module;
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <climits>
#include <cerrno>
#include <cstdlib>
#include <cstring>
export module apue;
import std;

template<typename... Args>
std::string format_message(const std::string& fmt, Args&&... args) {
	try {
		std::string user_msg = std::vformat(fmt, std::make_format_args(args...));
		return "Error: " + user_msg + ": " + strerror(errno);
	} catch (const std::format_error& e) {
		return "Formatting error: " + fmt + " (" + e.what() + ")";
	}
}

export {
	constexpr mode_t FILE_MODE = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	constexpr std::size_t PATH_MAX_GUESS = 1024;
	constexpr long OPEN_MAX_GUESS = 256;

	std::vector<char> path_alloc();

	long open_max();

	void set_fl(int fd, int flags);

	void clr_fl(int fd, int flags);

	/**
	 @brief Print user message + errno. Exit(1).
	*/
	template<typename... Args>
	void err_sys(const std::string& fmt, Args&&... args) {
		std::println(std::cerr, "{}", format_message(fmt, std::forward<Args>(args)...));
		std::exit(1);
	}

	/**
	 @brief Print user message + errno.
	*/
	template<typename... Args>
	void err_ret(const std::string& fmt, Args&&... args) {
		std::println(std::cerr, "{}", format_message(fmt, std::forward<Args>(args)...));
	}
}
