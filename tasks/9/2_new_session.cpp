//Напишите программу, которая с помощью функции fork порождает дочерний процесс, создающий, в свою очередь, новый сеанс.
// Проверьте, становится ли дочерний процесс лидером группы и теряет ли он управляющий терминал.

#include <apue.h>

void pr_ids(std::string name) {
	std::println("{}: pid = {}, ppid = {}, pgrp = {}, tpgrp = {}", name, getpid(), getppid(), getpgrp(), tcgetpgrp(STDIN_FILENO));
	fflush(stdout);
}

int main() {
	pid_t pid;
	pr_ids("parent");
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid == 0) { // child
		pr_ids("child before setsid");
		if (setsid() < 0) {
			err_sys("call setsid");
		}
		pr_ids("child after setsid");
	}
	exit(0);
}