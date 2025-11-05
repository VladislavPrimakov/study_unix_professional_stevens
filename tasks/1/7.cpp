// output usedid and groupid

import std;
#include <unistd.h>

int main(void) {
	std::println("uid = {}, gid = {}", getuid(), getgid());
	return 0;
}