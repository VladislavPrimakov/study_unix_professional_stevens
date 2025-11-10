// usage getpwnam

#include <apue.h>
#include <pwd.h>
#include <stddef.h>
#include <string.h> 

struct passwd* getpwnam(const char* name) {
	struct passwd* ptr;
	setpwent();
	while ((ptr = getpwent()) != NULL)
		if (strcmp(name, ptr->pw_name) == 0)
			break;
	endpwent();
	return(ptr);
}

void printpw(struct passwd* pw) {
	if (pw == NULL) {
		std::println("No such user");
		return;
	}
	std::println("name = {}", pw->pw_name);
	std::println("passwd = {}", pw->pw_passwd);
	std::println("uid = {}", pw->pw_uid);
	std::println("gid = {}", pw->pw_gid);
	std::println("gecos = {}", pw->pw_gecos);
	std::println("dir = {}", pw->pw_dir);
	std::println("shell = {}", pw->pw_shell);
}

int main() {
	struct passwd* r = getpwnam("root");
	printpw(r);
	return 0;
}