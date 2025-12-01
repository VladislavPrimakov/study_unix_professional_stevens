// check access to the file in argument

#include "apue.h"

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::println(std::cerr, "Using: {} <file_name>", argv[0]);
		return 1;
	}
	if (access(argv[1], R_OK) < 0)
		err_ret("Access file {}", argv[1]);
	else
		std::println("Access for reading is allowed");
	if (open(argv[1], O_RDONLY) < 0)
		err_ret("Open file {}", argv[1]);
	else
		std::println("File is opened for reading");
	return 0;
}
