#include "apue.h"
#include <ctype.h>
#include <fcntl.h>
#define BSZ 4096
unsigned char buf[BSZ];

unsigned char translate(unsigned char c) {
	if (isalpha(c)) {
		if (c >= 'n')
			c -= 13;
		else if (c >= 'a')
			c += 13;
		else if (c >= 'N')
			c -= 13;
		else
			c += 13;
	}
	return(c);
}

int main(int argc, char* argv[]) {
	int ifd, ofd, i, n;
	if (argc != 3)
		err_quit("Usage: {} infile outfile", argv[0]);
	if ((ifd = open(argv[1], O_RDONLY)) < 0)
		err_sys("call open {}", argv[1]);
	if ((ofd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, FILE_MODE)) < 0)
		err_sys("call create {}", argv[2]);
	while ((n = read(ifd, buf, BSZ)) > 0) {
		for (i = 0; i < n; i++)
			buf[i] = translate(buf[i]);
		if ((!writen(ofd, buf, n))) {
			err_sys("call write");
		}
	}
	fsync(ofd);
	return 0;
}