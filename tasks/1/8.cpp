//Чтение команд со стандартного ввода и их выполнение

#include "unistd.h"
#include <sys/wait.h>
#include "apue.h"

void sig_int(int signo) {
	printf("\nInterrupted\n%% ");
	fflush(stdout);
}

int main() {
	std::string line;
	pid_t pid;
	int status;

	if (signal(SIGINT, sig_int) == SIG_ERR) {
		throw UnixError("error call signal");
	}

	std::cout << "% " << std::flush;

	while (std::getline(std::cin, line)) {
		if (line.empty()) {
			std::cout << "% " << std::flush;
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
			throw UnixError("error call fork");
		} else if (pid == 0) { // child process
			execvp(argv_c[0], argv_c.data());

			throw UnixError("cannot execute: " + line);
			exit(127);
		}

		// parent process
		if (waitpid(pid, &status, 0) < 0) {
			throw UnixError("error call waitpid");
		}

		std::cout << "% " << std::flush;
	}

	return 0;
}