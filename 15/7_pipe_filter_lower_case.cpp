#include "apue.h"

int main() {
	char line[MAXLINE];
	FILE* fpin;
	constexpr char cmd[] = "./15_6_filter_lower_case";
	if ((fpin = popen(cmd, "r")) == NULL)
		err_sys("call popen {}", cmd);
	while (true) {
		fputs("prompt> ", stdout);
		fflush(stdout);
		if (fgets(line, MAXLINE, fpin) == NULL)
			break;
		if (fputs(line, stdout) == EOF)
			err_sys("call fputs");
	}
	if (pclose(fpin) == -1)
		err_sys("call pclose");
	putchar('\n');
	exit(0);
}