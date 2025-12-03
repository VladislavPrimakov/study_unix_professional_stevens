#include "apue.h"
#include <arpa/inet.h>
#include <netdb.h>


void print_family(struct addrinfo* aip) {
	std::string s = " family ";
	switch (aip->ai_family) {
	case AF_INET:
		s += "inet";
		break;
	case AF_INET6:
		s += "inet6";
		break;
	case AF_UNIX:
		s += "unix";
		break;
	case AF_UNSPEC:
		s += "unspecified";
		break;
	default:
		s += "unknow";
	}
	std::print("{}", s);
}

void print_type(struct addrinfo* aip) {
	std::string s = " type ";
	switch (aip->ai_socktype) {
	case SOCK_STREAM:
		s += "stream";
		break;
	case SOCK_DGRAM:
		s += "datagram";
		break;
	case SOCK_SEQPACKET:
		s += "seqpacket";
		break;
	case SOCK_RAW:
		s += "raw";
		break;
	default:
		s += "unknown " + std::to_string(aip->ai_socktype);
	}
	std::print("{}", s);
}

void print_protocol(struct addrinfo* aip) {
	std::string s = " protocol ";
	switch (aip->ai_protocol) {
	case 0:
		s += "default";
		break;
	case IPPROTO_TCP:
		s += "TCP";
		break;
	case IPPROTO_UDP:
		s += "UDP";
		break;
	case IPPROTO_RAW:
		s += "raw";
		break;
	default:
		s += "unknow " + aip->ai_protocol;
	}
	std::print("{}", s);
}

void print_flags(struct addrinfo* aip) {
	std::string s = " flags";
	if (aip->ai_flags == 0) {
		s += " 0";
	}
	else {
		if (aip->ai_flags & AI_PASSIVE)
			s += " passive";
		if (aip->ai_flags & AI_CANONNAME)
			s += " canon";
		if (aip->ai_flags & AI_NUMERICHOST)
			s += " numhost";
		if (aip->ai_flags & AI_NUMERICSERV)
			s += " numserv";
		if (aip->ai_flags & AI_V4MAPPED)
			s += " v4mapped";
		if (aip->ai_flags & AI_ALL)
			s += " all";
	}
	std::print("{}", s);
}

int main(int argc, char* argv[]) {
	struct addrinfo* ailist, * aip;
	struct addrinfo hint;
	struct sockaddr_in* sinp;
	const char* addr;
	int err;
	char abuf[INET_ADDRSTRLEN];
	if (argc != 3)
		err_quit("usage: {} host_name service", argv[0]);
	hint.ai_flags = AI_CANONNAME;
	hint.ai_family = 0;
	hint.ai_socktype = 0;
	hint.ai_protocol = 0;
	hint.ai_addrlen = 0;
	hint.ai_canonname = NULL;
	hint.ai_addr = NULL;
	hint.ai_next = NULL;
	if ((err = getaddrinfo(argv[1], argv[2], &hint, &ailist)) != 0)
		err_quit("call getaddrinfo: {}", gai_strerror(err));
	for (aip = ailist; aip != NULL; aip = aip->ai_next) {
		print_flags(aip);
		print_family(aip);
		print_type(aip);
		print_protocol(aip);
		std::print("\n\thost {}", aip->ai_canonname ? aip->ai_canonname : "-");
		if (aip->ai_family == AF_INET) {
			sinp = (struct sockaddr_in*)aip->ai_addr;
			addr = inet_ntop(AF_INET, &sinp->sin_addr, abuf, INET_ADDRSTRLEN);
			std::print(" address {}", addr ? addr : "unknown");
			std::print(" port {}", ntohs(sinp->sin_port));
		}
		std::println();
	}
	exit(0);
}