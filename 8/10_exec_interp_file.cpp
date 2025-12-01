#include "apue.h"
#include <pwd.h>

// $ cat /home/sar/bin/testinterp
// #!/home/sar/bin/echoarg foo
// echoarg from 7_3

int main(void) {
	pid_t pid;
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid == 0) {
		if (execl("/home/prima/bin/testinterp", "testinterp", "myarg1", "MY ARG2", (char*)0) < 0) {
			err_sys("call execl");
		}
	}
	if (waitpid(pid, NULL, 0) < 0)
		err_sys("call waitpid");
	exit(0);
}