// usage st_dev st_rdev

#include "apue.h"
#include <sys/sysmacros.h> 
#include <sys/types.h>

int main(int argc, char* argv[]) {
	struct stat buf;
	for (std::size_t i = 1; i < argc; i++) {
		std::print("{}: ", argv[i]);
		if (stat(argv[i], &buf) < 0) {
			err_ret("call stat");
			continue;
		}
		std::print("dev = {}/{}", major(buf.st_dev), minor(buf.st_dev));
		if (S_ISCHR(buf.st_mode) || S_ISBLK(buf.st_mode)) {
			std::print(" ({}) rdev = {}/{}", (S_ISCHR(buf.st_mode)) ? "sym device" : "block device", major(buf.st_rdev), minor(buf.st_rdev));
		}
		std::println();
	}
	return 0;
}