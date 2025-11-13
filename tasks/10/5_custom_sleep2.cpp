#include <apue.h>
#include <chrono>
#include <setjmp.h>

using namespace std::chrono_literals;

jmp_buf env_alrm;
unsigned int sleep2(unsigned int);
static void sig_int(int);

int main() {
	unsigned int unslept;
	if (signal(SIGINT, sig_int) == SIG_ERR)
		err_sys("call signal(SIGINT)");
	unslept = sleep2(5);
	std::println("func sleep2 returned value: {}", unslept);
	return 0;
}

void sig_int(int signo) {
	int i, j;
	volatile int k;
	std::println("funct sig_int started");
	auto start_time = std::chrono::steady_clock::now();
	while (std::chrono::steady_clock::now() - start_time < 5s) {
		for (int j = 0; j < 10000; j++) {
			k += i * j;
		}
		i++;
	}
	std::println("func sig_int finished");
}

void sig_alrm(int signo) {
	longjmp(env_alrm, 1);
}

unsigned int sleep2(unsigned int seconds) {
	if (signal(SIGALRM, sig_alrm) == SIG_ERR)
		return(seconds);
	if (setjmp(env_alrm) == 0) {
		alarm(seconds);
		pause();
	}
	return(alarm(0));
}