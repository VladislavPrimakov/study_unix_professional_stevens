#include <apue.h>

int main() {
	pr_mask("pid =  " + std::to_string(getpid()));
	return 0;
}