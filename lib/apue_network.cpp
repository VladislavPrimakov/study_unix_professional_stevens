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

int setup_server_ipv4(int port, int type, int qlen) {
	int sockfd;
	struct sockaddr_in serv_addr;
	if ((sockfd = socket(AF_INET, type, 0)) < 0) {
		return -1;
	}
	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; // IPv4
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		close(sockfd);
		return -1;
	}
	if (listen(sockfd, qlen) < 0) {
		close(sockfd);
		return -1;
	}
	return sockfd;
}