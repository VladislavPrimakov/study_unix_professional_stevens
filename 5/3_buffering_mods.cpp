// buffering modes of FILE objects

#include "apue.h"
#include <stdio.h>

void pr_stdio(const char*, FILE*);
int is_unbuffered(FILE*);
int is_linebuffered(FILE*);
int buffer_size(FILE*);

int main() {
	FILE* fp;
	fputs("Enter any symbol\n", stdout);
	if (getchar() == EOF)
		err_sys("call getchar");
	fputs("this line oputputs to sterr\n", stderr);
	pr_stdio("stdin", stdin);
	pr_stdio("stdout", stdout);
	pr_stdio("stderr", stderr);
	if ((fp = fopen("/etc/passwd", "r")) == NULL)
		err_sys("call fopen");
	if (getc(fp) == EOF)
		err_sys("call getc");
	pr_stdio("/etc/passwd", fp);
	return 0;
}

void pr_stdio(const char* name, FILE* fp) {
	printf("stream = %s, ", name);
	if (is_unbuffered(fp))
		printf("no buffering");
	else if (is_linebuffered(fp))
		printf("lined buffering");
	else
		printf("full buffering");
	printf(", bufsize = %d\n", buffer_size(fp));
}


// for libc 64 bit
int is_unbuffered(FILE* fp) {
	return (fp->_flags & _IONBF);
}

int is_linebuffered(FILE* fp) {
	return (fp->_flags & _IOLBF);
}

int buffer_size(FILE* fp) {
	if (fp->_IO_buf_base) {
		return (fp->_IO_buf_end - fp->_IO_buf_base);
	}
	return 0;
}