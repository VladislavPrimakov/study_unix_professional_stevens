#include "apue.h"

int connect_ipv4_host(const char* host, int port, int type) {
	struct addrinfo hints, * ailist, * aip;
	int sockfd, err;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = type;
	hints.ai_family = AF_INET;
	std::string port_str = std::to_string(port);
	if ((err = getaddrinfo(host, port_str.c_str(), &hints, &ailist)) != 0) {
		err_ret("getaddrinfo error: {}", gai_strerror(err));
		return err;
	}
	for (aip = ailist; aip != NULL; aip = aip->ai_next) {
		if ((sockfd = connect_ipv4_addr((struct sockaddr_in*)aip->ai_addr, 0, type)) >= 0) {
			freeaddrinfo(ailist);
			return sockfd;
		}
	}
	freeaddrinfo(ailist);
	return -1;
}

int connect_ipv4_addr(const struct sockaddr_in* addr, int port, int type) {
	int sockfd = setup_socket_ipv4(port, type);
	if (connect(sockfd, (struct sockaddr*)addr, sizeof(*addr)) < 0) {
		close(sockfd);
		return -1;
	}
	return sockfd;
}

int setup_socket_ipv4(int port, int type) {
	int sockfd;
	struct sockaddr_in serv_addr;
	if ((sockfd = socket(AF_INET, type, 0)) < 0) {
		return -1;
	}
	int opt = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		close(sockfd);
		return -1;
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
		close(sockfd);
		return -1;
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; // IPv4
	serv_addr.sin_port = (port > 0) ? htons(port) : 0;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		close(sockfd);
		return -1;
	}
	return sockfd;
}