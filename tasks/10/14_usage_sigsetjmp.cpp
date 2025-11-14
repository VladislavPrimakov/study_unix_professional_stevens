#include "apue.h"
#include <setjmp.h>
#include <time.h>

void sig_usr1(int);
void sig_alrm(int);
sigjmp_buf jmpbuf;
volatile sig_atomic_t canjump;

int main() {
	if (signal(SIGUSR1, sig_usr1) == SIG_ERR)
		err_sys("call signal(SIGUSR1)");
	if (signal(SIGALRM, sig_alrm) == SIG_ERR)
		err_sys("call signal(SIGALRM)");
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
	time_t starttime;
	if (canjump == 0)
		return; /* получен неожиданный сигнал, игнорировать */
	pr_mask("в начале функции sig_usr1: ");
	alarm(3); /* запланировать SIGALRM через 3 секунды */
	starttime = time(NULL);
	for (; ; ) /* ждать 5 секунд */
		if (time(NULL) > starttime + 5)
			break;
	pr_mask("в конце функции sig_usr1: ");
	canjump = 0;
	siglongjmp(jmpbuf, 1); /* переход в функцию main – не возврат */
}
static void
sig_alrm(int signo)
{
	pr_mask("в функции sig_alrm: ");
}