#include "apue.h"

int pfd1[2], pfd2[2];

void TELL_WAIT_PIPE() {
	if (pipe(pfd1) < 0 || pipe(pfd2) < 0)
		err_sys("TELL_WAIT_PIPE: call pipe");
}

void TELL_DONE_PIPE() {
	close(pfd1[0]);
	close(pfd1[1]);
	close(pfd2[0]);
	close(pfd2[1]);
}

void TELL_PARENT() {
	if (write(pfd2[1], "c", 1) != 1)
		err_sys("TELL_PARENT: call write");
}

void WAIT_PARENT() {
	char c;
	if (read(pfd1[0], &c, 1) != 1)
		err_sys("WAIT_PARENT: call read");
	if (c != 'p')
		err_quit("WAIT_PARENT: taken incorrect data");
}

void TELL_CHILD() {
	if (write(pfd1[1], "p", 1) != 1)
		err_sys("TELL_CHILD: call write");
}

void WAIT_CHILD() {
	char c;
	if (read(pfd2[0], &c, 1) != 1)
		err_sys("WAIT_CHILD: call read");
	if (c != 'c')
		err_quit("WAIT_CHILD: taken incorrect data");
}