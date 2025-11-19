// Напишите программу, с помощью которой можно было бы проверить функции синхронизации 
// родительского и дочернего процессов из листинга 10.17.Процесс должен создавать файл и записывать в него число 0. 
// Затем вызывать функцию fork, после чего родительский и дочерний процессы должны по очереди увеличивать число, 
// прочитанное из файла.При каждом увеличении счетчика процесс должен выводить информацию о том, кто произвел увеличение — родитель или потомок.

#include <apue.h>
#include <charconv>

void inc(const int& fd, const std::string& msg) {
	std::string s(10, '\0');
	lseek(fd, 0, SEEK_SET);
	int r = read(fd, s.data(), s.size());
	if (r < 0) {
		err_sys("read");
	}
	std::size_t num;
	auto [ptr, ec] = std::from_chars(s.data(), s.data() + r, num);
	if (ec == std::errc()) {
		num++;
		lseek(fd, 0, SEEK_SET);
		s = std::to_string(num);
		write(fd, s.data(), s.size());
		std::println("{} incremented to {}", msg, num);
	}
	else {
		err_quit("std::from_chars");
	}
}

int main() {
	int fd;
	std::size_t n = 10;
	if ((fd = open("tempfile", O_RDWR | O_CREAT | O_TRUNC, FILE_MODE)) < 0)
		err_sys("call open");
	int w = write(fd, "0", 1);
	if (w != 1) {
		err_sys("call write");
	}
	pid_t pid;
	TELL_WAIT();
	if ((pid = fork()) < 0) {
		err_sys("fork");
	}
	else if (pid == 0) { // Child process
		auto ppid = getppid();
		for (std::size_t i = 0; i < n; ++i) {
			// child process first
			inc(fd, "Child");
			TELL_PARENT(ppid);
			WAIT_PARENT();
		}
	}
	else { // Parent process
		for (std::size_t i = 0; i < n; ++i) {
			WAIT_CHILD();
			inc(fd, "Parent");
			TELL_CHILD(pid);
		}
	}
	TELL_DONE();
	exit(0);
}