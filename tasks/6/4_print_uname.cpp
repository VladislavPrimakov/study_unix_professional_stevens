// Напишите программу, которая вызывает функцию uname и выводит содержимое всех полей структуры utsname. 
// Сравните получившиеся результаты с тем, что выводит команда uname(1).

#include <apue.h>
#include <sys/utsname.h>

void print_utsname(const struct utsname& uts) {
	std::println("System name: {}", uts.sysname);
	std::println("Node name: {}", uts.nodename);
	std::println("Release: {}", uts.release);
	std::println("Version: {}", uts.version);
	std::println("Machine: {}", uts.machine);
}

int main() {
	struct utsname uts;
	if (uname(&uts) == -1) {
		err_sys("uname error");
	}
	print_utsname(uts);

	return 0;
}