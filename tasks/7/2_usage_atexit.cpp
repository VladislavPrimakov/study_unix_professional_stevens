// usage of atexit() to register exit handlers

#include "apue.h"
static void my_exit1(void);
static void my_exit2(void);

int main(void) {
	if (atexit(my_exit2) != 0)
		err_sys("call atexit my_exit2");
	if (atexit(my_exit1) != 0)
		err_sys("call atexit my_exit1");
	if (atexit(my_exit1) != 0)
		err_sys("call atexit my_exit1");
	std::println("main finished");
	return(0);
}

void my_exit1(void) {
	std::println("first exit handler");
}

void my_exit2(void) {
	std::println("second exit handler");
}