#include "apue.h"

int main() {
	int status;
	if ((status = system("date")) < 0)
		err_sys("call system()");
	pr_exit(status);
	if ((status = system("nosuchcommand")) < 0)
		err_sys("call system()");
	pr_exit(status);
	if ((status = system("who; exit 44")) < 0)
		err_sys("call system()");
	pr_exit(status);
	return 0;
}