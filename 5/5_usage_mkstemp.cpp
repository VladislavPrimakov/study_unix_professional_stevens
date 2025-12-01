// usage mksktemp

#include "apue.h"
#include <errno.h>
void make_temp(std::string tmplate);

int main() {
	std::string good_template = "/tmp/dirXXXXXX";
	std::println("ettemp to create first temp file...");
	make_temp(good_template);
	printf("attemp to create second temp file...");
	make_temp(good_template);
	return 0;
}

void make_temp(std::string tmplate) {
	int fd;
	struct stat sbuf;
	if ((fd = mkstemp(tmplate.data())) < 0)
		err_sys("call mkstemp");
	std::println("temp name = {}", tmplate);
	close(fd);
	if (stat(tmplate.data(), &sbuf) < 0) {
		if (errno == ENOENT)
			std::println("file not exist");
		else
			err_sys("call stat");
	}
	else {
		std::println("file exists");
		unlink(tmplate.data());
	}
}