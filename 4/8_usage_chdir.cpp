// usage chdir

#include "apue.h"

int main(void)
{
	if (chdir("/tmp") < 0)
		err_sys("call chdir");
	std::println("directory /tmp has become cwd");
	exit(0);
}