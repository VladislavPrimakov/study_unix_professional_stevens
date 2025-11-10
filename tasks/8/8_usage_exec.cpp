#include "apue.h"
#include <pwd.h>

const char* env_init[] = { "USER=unknown", "PATH=/tmp", NULL };

std::string getpwnam() {
	struct passwd* user_info = getpwuid(getuid());
	return std::string(user_info->pw_name);
}

int main() {
	pid_t pid;
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid == 0) { /* set fullpath and env */
		std::println("\nusing execle to run echoall");
		std::string p = "/home/" + getpwnam() + "/bin/echoall";
		if (execle(p.c_str(), "echoall", "myarg1", "MY ARG2", (char*)0, env_init) < 0) {
			err_sys("call execle");
		}
	}
	if (waitpid(pid, NULL, 0) < 0) {
		err_sys("call wait");
	}
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid == 0) { /* set filename */
		std::println("\nusing execlp to run echoall");
		if (execlp("echoall", "echoall", "only one argument", (char*)0) < 0) {
			err_sys("call execlp");
		}
	}
	return 0;
}