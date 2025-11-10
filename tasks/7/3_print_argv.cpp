// usage of argc and argv to print command-line arguments

#include "apue.h"

int main(int argc, char* argv[]) {
	for (std::size_t i = 0; i < argc; i++)
		std::println("argv[{}]: {}", i, argv[i]);
	return 0;
}