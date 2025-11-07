// usage umask

#include <apue.h>

#define RWRWRW (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

int main() {
	umask(0);
	if (creat("foo", RWRWRW) < 0)
		err_sys("creat file \"foo\"");
	umask(S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (creat("bar", RWRWRW) < 0)
		err_sys("creat file \"bar\"");
	return 0;
}
