// read commands from stdin and execution them

import apue;
import std;
#include <sys/wait.h>


void sig_int(int signo) {
	std::println("\nInterrupted");
	std::print("% ");
}

int main() {
	std::string line;
	pid_t pid;
	int status;

	if (signal(SIGINT, sig_int) == SIG_ERR) {
		err_sys("error call signal");
	}

	std::print("% ");

	while (std::getline(std::cin, line)) {
		std::print("% ");
		if (line.empty()) {
			continue;
		}

		std::stringstream ss(line);
		std::string arg;
		std::vector<std::string> args_storage;
		while (ss >> arg) {
			args_storage.push_back(arg);
		}
		std::vector<char*> argv_c(args_storage.size() + 1);
		for (size_t i = 0; i < args_storage.size(); ++i) {
			argv_c[i] = args_storage[i].data();
		}
		if ((pid = fork()) < 0) {
			err_sys("error call fork");
		} else if (pid == 0) {
			// child process
			execvp(argv_c[0], argv_c.data());

			err_sys("cannot execute: {}", line);
			return 127;
		}
		// parent process
		if (waitpid(pid, &status, 0) < 0) {
			err_sys("error call waitpid");
		}
	}
	return 0;
}
