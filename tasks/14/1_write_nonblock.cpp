#include "apue.h"

std::vector<char> buf(500000);

int main() {
	int ntowrite, nwritten;
	ntowrite = read(STDIN_FILENO, buf.data(), buf.size());
	std::println(stderr, "read {} bytes", ntowrite);
	char* ptr = buf.data();
	set_fl(STDOUT_FILENO, O_NONBLOCK);
	while (ntowrite > 0) {
		nwritten = write(STDOUT_FILENO, ptr, ntowrite);
		std::println(stderr, "nwrite = {}, errno = {}", nwritten, errno);
		if (nwritten > 0) {
			ptr += nwritten;
			ntowrite -= nwritten;
		}
	}
	clr_fl(STDOUT_FILENO, O_NONBLOCK);
	exit(0);
}