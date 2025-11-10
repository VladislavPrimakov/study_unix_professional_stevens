#include <apue.h>
#include <stdio.h>

FILE* open_data(void) {
	FILE* fp;
	char databuf[BUFSIZ];
	if ((fp = fopen("datafile", "r")) == NULL)
		return(NULL);
	if (setvbuf(fp, databuf, _IOLBF, BUFSIZ) != 0)
		return(NULL);
	return(fp); // error
}

int main() {
	FILE* file = open_data();
	// databuf goes out of scope here, so file's buffer is invalid
	if (file != NULL) {
		fclose(file);
	}
	return 0;
}