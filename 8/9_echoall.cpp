#include "apue.h"


int main(int argc, char* argv[]) {
	char** ptr;
	extern char** environ;
	for (std::size_t i = 0; i < argc; i++)
		std::println("argv[{}]: {}", i, argv[i]);
	for (ptr = environ; *ptr != NULL; ptr++)
		std::println("{}", *ptr);
	exit(0);
}