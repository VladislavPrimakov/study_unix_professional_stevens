// usage strftime

#include "apue.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
	time_t t;
	struct tm* tmp;
	char buf1[16];
	char buf2[64];
	time(&t);
	tmp = localtime(&t);
	if (strftime(buf1, 16, "time and date: %r, %a %b %d, %Y", tmp) == 0)
		std::println("buff length 16 is too small");
	else
		std::println("{}", buf1);
	if (strftime(buf2, 64, "time and date: %r, %a %b %d, %Y", tmp) == 0)
		std::println("buff length 64 is too small");
	else
		std::println("{}", buf2);
	return 0;
}