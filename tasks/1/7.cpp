// Вывод идентификаторов пользователя и группы

#include <unistd.h>
#include <cstdlib>
#include <stdio.h>

int main(void) {
	printf("uid = %d, gid = %d\n", getuid(), getgid());
	exit(0);
}