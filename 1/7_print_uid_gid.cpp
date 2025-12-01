// output usedid and groupid

#include "apue.h"

int main(void) {
	std::println("uid = {}, gid = {}", getuid(), getgid());
	return 0;
}