// example usage of open_max() function from apue.h

#include <apue.h>

int main() {
	long max_files = open_max();
	std::println("Maximum open files limit: {}", max_files);
	std::println("Calling again (cached): {}", open_max());
	return 0;
}
