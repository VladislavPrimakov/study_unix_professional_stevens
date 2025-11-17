#include "apue.h"
#include <chrono>
#include <setjmp.h>

using namespace std::chrono_literals;

void sig_usr1(int);
void sig_alrm(int);
sigjmp_buf jmpbuf;
volatile sig_atomic_t canjump;

int main() {
	if (apue_signal(SIGUSR1, sig_usr1) == SIG_ERR) {
		err_sys("call apue_signal(SIGUSR1)");
	}
	if (apue_signal(SIGALRM, sig_alrm) == SIG_ERR) {
		err_sys("call apue_signal(SIGALRM)");
	}
	pr_mask("at the beggining of main: ");
	if (sigsetjmp(jmpbuf, 1)) {
		pr_mask("at the end of main: ");
		exit(0);
	}
	canjump = 1;
	for (; ; )
		pause();
}

void sig_usr1(int signo) {
	if (canjump == 0)
		return;
	pr_mask("at the beginning of sig_usr1: ");
	alarm(3);
	auto starttime = std::chrono::steady_clock::now();
	for (; ; ) {
		if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - starttime) > 5s) {
			break;
		}
	}
	pr_mask("at the end of sig_usr1: ");
	canjump = 0;
	siglongjmp(jmpbuf, 1);
}

void sig_alrm(int signo) {
	pr_mask("at sig_alrm: ");
}