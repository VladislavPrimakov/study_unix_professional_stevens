// usage unlink

#include <apue.h>

int main() {
	if (open("tempfile", O_RDWR | O_CREAT) < 0)
		err_sys("call open for file tempfile");
	sleep(15);
	if (unlink("tempfile") < 0)
		err_sys("call unlink for file tempfile");
	std::println("file removed");
	sleep(15);
	std::println("end");
	return 0;
}