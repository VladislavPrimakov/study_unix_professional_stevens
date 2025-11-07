// example usage of path_alloc() function from apue

#include <apue.h>

int main(int argc, char* argv[]) {
	std::println("Allocating path buffer...");
	auto path_buffer = path_alloc();
	std::println("Successfully allocated buffer.");
	std::println("Buffer size (from vector.size()): {}", path_buffer.size());
	if (getcwd(path_buffer.data(), path_buffer.size()) != nullptr) {
		std::println("Current working directory: {}", path_buffer.data());
	}
	else {
		err_ret("getcwd error");
	}
	return 0;
}
