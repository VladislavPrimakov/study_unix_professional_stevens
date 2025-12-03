#include "apue.h"

int connect_to_server(const char* host, int port, int type, int maxsleep) {
	struct addrinfo hint, * ailist, * aip;
	int sockfd, err;
	memset(&hint, 0, sizeof(hint));
	hint.ai_socktype = type;
	hint.ai_family = AF_UNSPEC; // IPv4 или IPv6
	std::string port_str = std::to_string(port);
	if ((err = getaddrinfo(host, port_str.c_str(), &hint, &ailist)) != 0) {
		err_quit("call getaddrinfo: {}", gai_strerror(err));
	}
	for (aip = ailist; aip != NULL; aip = aip->ai_next) {
		for (int numsec = 1; numsec <= maxsleep; numsec <<= 1) {
			if ((sockfd = socket(aip->ai_family, type, 0)) < 0) {
				// try the next address
				break;
			}
			if (connect(sockfd, aip->ai_addr, aip->ai_addrlen) == 0) {
				// success
				freeaddrinfo(ailist);
				return sockfd;
			}
			// failure
			close(sockfd);
			if (numsec <= maxsleep / 2) {
				std::println(stderr, "Connection failed, retrying in {} sec...", numsec);
				sleep(numsec);
			}
		}
	}
	freeaddrinfo(ailist);
	return -1;
}

int initserver(int port, int type, int qlen) {
	struct addrinfo hint, * ailist, * aip;
	int sockfd, err;
	memset(&hint, 0, sizeof(hint));
	hint.ai_flags = AI_PASSIVE;     // (0.0.0.0)
	hint.ai_socktype = type;
	hint.ai_canonname = NULL;
	hint.ai_addr = NULL;
	hint.ai_next = NULL;
	std::string port_str = std::to_string(port);
	if ((err = getaddrinfo(NULL, port_str.c_str(), &hint, &ailist)) != 0) {
		syslog(LOG_ERR, "ruptimed: getaddrinfo error: %s", gai_strerror(err));
		return -1;
	}
	for (aip = ailist; aip != NULL; aip = aip->ai_next) {
		if ((sockfd = socket(aip->ai_family, type, 0)) < 0) {
			continue;
		}
		int opt = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
			close(sockfd);
			continue;
		}
		if (bind(sockfd, aip->ai_addr, aip->ai_addrlen) < 0) {
			close(sockfd);
			continue;
		}
		if (type == SOCK_STREAM || type == SOCK_SEQPACKET) {
			if (listen(sockfd, qlen) < 0) {
				close(sockfd);
				continue;
			}
		}
		freeaddrinfo(ailist);
		return sockfd;
	}
	freeaddrinfo(ailist);
	return -1;
}