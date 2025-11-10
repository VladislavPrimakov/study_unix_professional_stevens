//5.1 Напишите реализацию функции setbuf с использованием функции setvbuf.

#include "apue.h"

// if buf is NULL, the stream is unbuffered
// else make it fully buffered with a buffer size of BUFSIZ
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
		printf("This is a test string to demonstrate mysetbuf function.\n");
	}
	auto t2 = rdtsc();
	std::println(stderr, "Time for fully buffered = {}", t2 - t1);
	mysetbuf(stdout, NULL);
	auto t3 = rdtsc();
	for (std::size_t i = 0; i < 100000; ++i) {
		printf("This is a test string to demonstrate mysetbuf function.\n");
	}
	auto t4 = rdtsc();
	std::println(stderr, "Time for unbuffered = {}", t4 - t3);
	std::println(stderr, "Difference = {}", (double)(t4 - t3) / (t2 - t1));
	return 0;
}