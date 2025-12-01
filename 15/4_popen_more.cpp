#include "apue.h"

constexpr const char* PAGER = "${PAGER:-more}";

int main(int argc, char* argv[]) {
	char line[MAXLINE];
	FILE* fpin, * fpout;
	if (argc != 2)
		err_quit("Usage: {}  <filepath>");
	if ((fpin = fopen(argv[1], "r")) == NULL)
		err_sys("can't open {}", argv[1]);
	if ((fpout = popen(PAGER, "w")) == NULL)
		err_sys("call popen");
	while (fgets(line, MAXLINE, fpin) != NULL) {
		if (fputs(line, fpout) == EOF)
			err_sys("call fputs to pipe");
	}
	if (ferror(fpin))
		err_sys("call fgets");
	if (pclose(fpout) == -1)
		err_sys("call pclose");
	exit(0);
}