#ifndef _APUE_H
#define _APUE_H

#include <iostream>       // For std::cout, std::cin, std::cerr, std::endl, std::flush
#include <string>         // For std::string, std::getline
#include <vector>         // For std::vector
#include <sstream>        // For std::stringstream
#include <stdexcept>      // For std::runtime_error
#include <csignal>        // For signal, SIGINT, SIG_ERR
#include <cstdlib>        // For exit
#include <cstring>        // For strerror
#include <cerrno>         // For errno

#define MAXLINE 4096

class UnixError : public std::runtime_error {
public:
	UnixError(const std::string& msg) : std::runtime_error(msg + ": " + strerror(errno)) {}
};

#endif /* _APUE_H */