#include "apue.h"

int main(int argc, char* argv[]) {
	int status;
	if (argc < 2)
		err_quit("usage {} <arg>", argv[0]);
	if ((status = system(argv[1])) < 0)
		err_sys("call system");
	pr_exit(status);
	std::println("real uid = {}, effective uid = {}", getuid(), geteuid());

	return 0;
}