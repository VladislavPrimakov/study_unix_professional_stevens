#include "apue.h"

int main(int argc, char* argv[]) {
	daemonize(argv[0]);
	// Daemon code here
	while (true) {
		pause();
	}
	return 0;
}