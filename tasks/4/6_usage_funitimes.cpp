// usage futimens

#include <apue.h>

int main(int argc, char* argv[]) {
	int i, fd;
	struct stat statbuf;
	struct timespec times[2];
	for (i = 1; i < argc; i++) {
		if (stat(argv[i], &statbuf) < 0) {
			err_ret("call stat for file {}", argv[i]);
			continue;
		}
		if ((fd = open(argv[i], O_RDWR | O_TRUNC)) < 0) {
			err_ret("call open for file {}", argv[i]);
			continue;
		}
		times[0] = statbuf.st_atim;
		times[1] = statbuf.st_mtim;
		if (futimens(fd, times) < 0) {
			err_ret("call futimens for file", argv[i]);
			close(fd);
		}
	}
	return 0;
}