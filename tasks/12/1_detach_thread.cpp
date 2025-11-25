#include "apue.h"
#include <pthread.h>

int main() {
	auto thread_function = [](void* arg) -> void* {
		int* num = static_cast<int*>(arg);
		std::println("Thread number: {}", *num);
		return nullptr;
		};
	int thread_arg = 42;
	if (makethread(thread_function, &thread_arg) != 0) {
		err_quit("Failed to create detached thread");
	}
	sleep(1);
	return 0;
}