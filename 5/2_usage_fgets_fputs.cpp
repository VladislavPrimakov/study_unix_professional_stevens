// usage : fgets_fputs < inputfile > outputfile

#include "apue.h"


int main() {
	char buf[MAXLINE];
	while (fgets(buf, MAXLINE, stdin) != NULL)
		if (fputs(buf, stdout) == EOF)
			err_sys("call fputs");
	if (ferror(stdin))
		err_sys("call fgets");
	exit(0);
}