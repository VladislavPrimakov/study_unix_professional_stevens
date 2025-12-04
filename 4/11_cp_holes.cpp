//Напишите утилиту, аналогичную cp(1), которая копировала бы файлы с дырками, не записывая байты со значением 0 в выходной файл.

#include "apue.h"
#include <algorithm>

int main(int argc, char* argv[]) {
	if (argc < 3) {
		err_quit("Usage: {} <file> ... <directory>", argv[0]);
	}
	if (mkdir(argv[argc - 1], FILE_MODE) < 0) {
		if (errno != EEXIST) {
			err_sys("call mkdir for {}", argv[argc - 1]);
		}
	}

	std::size_t BUFF_SIZE = 4096;
	std::vector<char> buf_read(BUFF_SIZE);
	for (std::size_t i = 1; i < static_cast<std::size_t>(argc) - 1; i++) {
		int fd_src = open(argv[i], O_RDONLY);
		if (fd_src < 0) {
			err_ret("call open source file {}", argv[i]);
			continue;
		}
		std::string dest_path = std::string(argv[argc - 1]) + "/" + std::string(argv[i]);
		int fd_dest = open(dest_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, FILE_MODE);
		if (fd_dest < 0) {
			err_ret("call open destination file {}", dest_path);
			continue;
		}
		std::size_t total_size = 0;
		while (std::size_t r = read(fd_src, buf_read.data(), BUFF_SIZE)) {
			if (r < 0) {
				err_sys("call read from source file {}", argv[i]);
			}
			std::size_t written = 0;
			while (written < r) {
				auto it_end = buf_read.begin() + r;
				auto it_first_zero = std::find(buf_read.begin() + written, it_end, '\0');
				std::size_t non_zero_size = std::distance(buf_read.begin() + written, it_first_zero);
				// write non-zero data
				if (non_zero_size > 0) {
					if (!writen(fd_dest, buf_read.data() + written, non_zero_size)) {
						err_sys("call write to destination file");
					}
				}
				// if first zero is at the end - break
				if (it_first_zero == it_end) {
					written += non_zero_size;
					break;
				}
				auto it_first_nonzero = std::find_if(it_first_zero, it_end, [](const char& c) { return c != '\0'; });
				// write zeros as a hole
				std::size_t zero_size = std::distance(it_first_zero, it_first_nonzero);
				if (lseek(fd_dest, zero_size, SEEK_CUR) < 0) {
					err_ret("call lseek in destination file {}", dest_path);
				}
				written += non_zero_size + zero_size;
			}
			total_size += r;
		}
		if (ftruncate(fd_dest, total_size) < 0) {
			err_ret("call ftruncate for destination file {}", dest_path);
		}
		close(fd_src);
		close(fd_dest);
	}
	return 0;
}

