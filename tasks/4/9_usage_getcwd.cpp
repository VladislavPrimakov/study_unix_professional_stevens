// usage getcwd

#include "apue.h"

int main(void) {
	if (chdir("/home") < 0)
		err_sys("call chdir");
	auto path = path_alloc();
	if (!getcwd(path.data(), path.size()))
		err_sys("call getcwd");
	std::println("cwd = {}", path.data());
	return 0;
}