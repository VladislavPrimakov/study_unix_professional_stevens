//5.1 Напишите реализацию функции setbuf с использованием функции setvbuf.

#include <apue.h>

void mysetbuf(FILE* file, char* buf) {
	if (setvbuf(file, buf, buf ? _IOFBF : _IONBF, BUFSIZ) < 0) {
		err_sys("call setvbuf");
	}
}

int main() {
	char buf[BUFSIZ];
	mysetbuf(stdout, buf);
	auto t1 = rdtsc();
	for (std::size_t i = 0; i < 100000; ++i) {
		std::println("This is a test string to demonstrate mysetbuf function.");
	}
	auto t2 = rdtsc();
	std::println(stderr, "Time for fully buffered = {}", t2 - t1);
	auto t3 = rdtsc();
	for (std::size_t i = 0; i < 100000; ++i) {
		std::println("This is a test string to demonstrate mysetbuf function.");
	}
	auto t4 = rdtsc();
	std::println(stderr, "Time for unbuffered = {}", t4 - t3);
	std::println(stderr, "Difference = {}", (double)(t4 - t3) / (t2 - t1));
	return 0;
}