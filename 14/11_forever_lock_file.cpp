//Напишите тестовую программу, демонстрирующую поведение вашей системы в ситуации, 
// когда процесс пытается получить блокировку для записи на участок файла, 
// на который уже установлена блокировка для чтения, и при этом продолжают 
// поступать запросы на установку блокировки для чтения.Будет ли подвешен процесс, 
// запросивший блокировку для записи, процессами, которые устанавливают блокировки для чтения ?

#include <apue.h>
#include <pthread.h>
using namespace std::chrono_literals;

template <typename Rep, typename Period>
void child_fn(int fd, int times, std::chrono::duration<Rep, Period> d) {
	for (int i = 0; i < times; i++) {
		if (readw_lock(fd, 0, SEEK_SET, 0) == -1) {
			err_sys("call read_lock for");
		}
		TELL_PARENT(getppid());
		std::println("[{}/{}] Child {} set read_lock", i, times, getpid());
		struct timespec ts = to_timespec(d);
		nanosleep(&ts, NULL);
		if (un_lock(fd, 0, SEEK_SET, 0) == -1) {
			err_sys("call un_lock");
		}
		std::println("[{}/{}] Child {} unset read_lock", i, times, getpid());
	}
	exit(0);
}

int main() {
	std::string filepath = "testfile.txt";
	int fd = open(filepath.c_str(), O_RDWR | O_CREAT, FILE_MODE);
	if (fd == -1) {
		err_sys("call open for {}", filepath.c_str());
	}

	TELL_WAIT();
	pid_t pid;
	if ((pid = fork()) < 0) {
		err_sys("call fork");
	}
	else if (pid == 0) { // child 1
		child_fn(fd, 3, 0.3s);
	}
	else { // parent
		if ((pid = fork()) < 0) {
			err_sys("call fork");
		}
		else if (pid == 0) { // child 2
			child_fn(fd, 3, 1s);
		}
		else { // parent
			// try to lock for write after child locked for read
			WAIT_CHILD();
			if (writew_lock(fd, 0, SEEK_SET, 0) == -1) {
				err_sys("parent: call writew_lock for {}", filepath.c_str());
			}
			std::println("Parent set write_lock");
			if (un_lock(fd, 0, SEEK_SET, 0) == -1) {
				err_sys("parent: call un_lock for {}", filepath.c_str());
			}
			std::println("Parent unset write_lock");
		}
	}
	int status;
	while ((pid = wait(&status)) > 0) {
		std::println("Child {} finished", pid);
	}
	TELL_DONE();
	exit(0);
}