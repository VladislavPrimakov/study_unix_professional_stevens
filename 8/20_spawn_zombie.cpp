//Напишите программу, которая создает процесс - зомби и 
// затем с помощью функции system запускает команду ps(1), 
// чтобы проверить, действительно ли процесс превратился в зомби.

#include "apue.h"

int main() {
	pid_t pid;
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid == 0) {
		// Child process: exit immediately to become a zombie
		_exit(0);
	}
	else {
		// Parent process: wait a moment to ensure child becomes a zombie
		sleep(1);
		std::string command = "ps -o pid,ppid,state,cmd | grep " + std::to_string(pid);
		if (system(command.c_str()) < 0) {
			err_sys("call system {}", command);
		}
	}
	return 0;
}