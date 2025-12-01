// usage volatile with setjmp/longjmp


#include "apue.h"
#include <setjmp.h>
static void f1(int, int, int);
static void f2(void);
static jmp_buf jmpbuffer;
static int globval;

int main() {
	int autoval;
	volatile int volaval;
	static int statval;
	globval = 1; autoval = 2; volaval = 3; statval = 4;
	if (setjmp(jmpbuffer) != 0) {
		std::println("after longjmp:");
		printf("globval = %d, autoval = %d, volaval = %d, statval = %d\n", globval, autoval, volaval, statval);
		return 0;
	}
	globval = 95; autoval = 96; volaval = 98; statval = 99;
	f1(autoval, volaval, statval);
	return 0;
}

void f1(int i, int j, int k) {
	std::println("into function f1():");
	printf("globval = %d, autoval = %d, volaval = %d, statval = %d\n", globval, i, j, k);
	f2();
}

void f2(void) {
	longjmp(jmpbuffer, 1);
}