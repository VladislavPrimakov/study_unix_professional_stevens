// usage tmpnam and tmpfile functions

#include "apue.h"

int main(void) {
	char name[L_tmpnam], line[MAXLINE];
	FILE* fp;
	std::println("{}", tmpnam(NULL));
	if (tmpnam(name) == NULL) {
		err_sys("call tmpnam");
	}
	std::println("{}", name);
	if ((fp = tmpfile()) == NULL)
		err_sys("call tmpfile");
	fputs("written line\n", fp);
	rewind(fp);
	if (fgets(line, sizeof(line), fp) == NULL)
		err_sys("call fgets");
	fputs(line, stdout);
	return 0;
}