#include "apue.h"

int main() {
	std::println("real uid = {}, effective uid = {}", getuid(), geteuid());
	return 0;
}