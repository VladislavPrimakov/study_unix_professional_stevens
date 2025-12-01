// usage gec/putc to copy input to output, with error checking

#include "apue.h"

int main() {
	int c;
	while ((c = getc(stdin)) != EOF)
		if (putc(c, stdout) == EOF)
			err_sys("call putc");
	if (ferror(stdin))
		err_sys("call getc");
	return 0;
}