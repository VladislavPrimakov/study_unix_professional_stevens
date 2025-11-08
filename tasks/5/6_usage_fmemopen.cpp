// usage fmemopen

#include "apue.h"
#define BSZ 48
int main() {
	FILE* fp;
	char buf[BSZ];
	memset(buf, 'a', BSZ - 2);
	buf[BSZ - 2] = '\0';
	buf[BSZ - 1] = 'X';
	if ((fp = fmemopen(buf, BSZ, "w+")) == NULL)
		err_sys("call fmemopen");
	std::println("init buffer consists: {}\n", buf);
	fprintf(fp, "Hello World");
	std::println("before flush: {}", buf);
	fflush(fp);
	std::println("after fflush: {}", buf);
	std::println("string length buffer = {}", strlen(buf));
	memset(buf, 'b', BSZ - 2);
	buf[BSZ - 2] = '\0';
	buf[BSZ - 1] = 'X';
	fprintf(fp, "Hello World");
	fseek(fp, 0, SEEK_SET);
	std::println("after fseek: {}", buf);
	std::println("string length buffer = {}", strlen(buf));
	memset(buf, 'c', BSZ - 2);
	buf[BSZ - 2] = '\0';
	buf[BSZ - 1] = 'X';
	fprintf(fp, "Hello World");
	fclose(fp);
	std::println("after fclose: {}", buf);
	std::println("string length buffer = {}", strlen(buf));
	return(0);
}