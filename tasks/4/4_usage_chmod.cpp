// usage chmod

#include <apue.h>

int main() {
	struct stat	statbuf;
	if (stat("foo", &statbuf) < 0)
		err_sys("call stat for file foo");
	if (chmod("foo", (statbuf.st_mode & ~S_IXGRP) | S_ISGID) < 0)
		err_sys("call chmod for file foo");
	if (chmod("bar", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0)
		err_sys("call chmod for file bar");
	return 0;
}