#include <apue.h>
#include <memory>
#include <pwd.h>

void my_alarm(int signo) {
	struct passwd* rootptr;
	std::println("inside SIGALRM handler");
	if ((rootptr = getpwnam("root")) == NULL)
		err_sys("call getpwnam(root)");
	alarm(1);
}

int main() {
	struct passwd* ptr;
	char* name = getlogin();
	signal(SIGALRM, my_alarm);
	alarm(1);
	for (; ; ) {
		if ((ptr = getpwnam(name)) == NULL) {
			err_sys("call getpwnam({})", name);
		}
		if (strcmp(ptr->pw_name, name) != 0)
			std::println("passwd->pw_name = {}", ptr->pw_name);
	}
}