#include "apue.h"
#include <sys/mman.h>

#define ARRAY_SIZE 40000
#define MALLOC_SIZE 100000
#define SHM_SIZE 100000
#define SHM_MODE 0600
#define SHM_NAME "/test_posix_shm"

char array[ARRAY_SIZE]; // bss segment

int main() {
	int shmfd;
	char* ptr;
	void* shmptr;

	// 1. Static (BSS)
	std::println("array[] from {} to {}", (void*)&array[0], (void*)&array[ARRAY_SIZE]);

	// 2. Stack
	std::println("stack approximately {}", (void*)&shmfd);

	// 3. (Heap)
	if ((ptr = (char*)malloc(MALLOC_SIZE)) == NULL)
		err_sys("call malloc");
	std::println("dynamic allocated area from {} to {}", (void*)ptr, (void*)(ptr + MALLOC_SIZE));

	// --- POSIX SHARED MEMORY START ---
	if ((shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, SHM_MODE)) < 0)
		err_sys("call shm_open");

	if (ftruncate(shmfd, SHM_SIZE) == -1)
		err_sys("call ftruncate");

	if ((shmptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0)) == MAP_FAILED)
		err_sys("call mmap");

	close(shmfd);

	std::println("segment shm joined from {} to {}", (void*)shmptr, (void*)((char*)shmptr + SHM_SIZE));

	if (shm_unlink(SHM_NAME) < 0)
		err_sys("call shm_unlink");

	munmap(shmptr, SHM_SIZE);

	exit(0);
}