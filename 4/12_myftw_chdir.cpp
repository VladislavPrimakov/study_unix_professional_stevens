//Наша версия функции ftw никогда не покидает текущий каталог.
//Измените эту функцию так, чтобы каждый раз, встречая каталог, 
//она вызывала функцию chdir для перехода в этот каталог и передавала функции lstat не полный путь к файлу, 
//а только его имя.После обработки всех файлов в каталоге произведите вызов chdir("..").Сравните время работы этих двух версий.

#include  <apue.h>
#include <cstddef>
#include <dirent.h>
#include <filesystem>
#include <memory>
#include <new>
#include <optional>

struct nfiles {
	long nreg{}, ndir{}, nblk{}, nchr{}, nfifo{}, nslink{}, nsock{}, ntot{};
	void printValue(const std::string s, long value) {
		std::println("{:<20}= {:<10} {:5.2f} %", s, value, value * 100.0 / ntot);
	}
	void printStat() {
		ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
		if (ntot == 0)
			ntot = 1;
		printValue("Regular files", nreg);
		printValue("Directories", ndir);
		printValue("Block special", nblk);
		printValue("Char special", nchr);
		printValue("FIFOs", nfifo);
		printValue("Symbolic links", nslink);
		printValue("Sockets", nsock);
	}
};

namespace fs = std::filesystem;
void myftw(std::string pathname, std::optional<struct stat> st, nfiles& nf);

int main(int argc, char* argv[]) {
	try {
		if (argc != 2)
			err_quit("Using {}: ftw <base_file>", argv[0]);
		nfiles nf;
		std::string s;
		s.reserve(1024);
		s = argv[1];
		if (chdir(s.c_str()) < 0) {
			err_sys("call chdir for {}", s);
		}
		myftw(s, std::optional<struct stat>(), nf);
		nf.printStat();
	}
	catch (const std::bad_alloc& e) {
		err_quit("{}", e.what());
	}
	return 0;
}

void myftw(std::string pathname, std::optional<struct stat> st, nfiles& nf) {
	if (!st.has_value()) {
		struct stat local_st;
		if (lstat(pathname.c_str(), &local_st) < 0) {
			err_ret("call stat for {}", pathname);
			return;
		}
		st = local_st;
	}
	if (!S_ISDIR(st.value().st_mode)) {
		switch (st.value().st_mode & S_IFMT) {
		case S_IFREG: nf.nreg++; break;
		case S_IFBLK: nf.nblk++; break;
		case S_IFCHR: nf.nchr++; break;
		case S_IFIFO: nf.nfifo++; break;
		case S_IFLNK: nf.nslink++; break;
		case S_IFSOCK: nf.nsock++; break;
		case S_IFDIR:
			err_dump("S_IFDIR for {}", pathname);
		}
		return;
	}

	nf.ndir++;
	DIR* dir_raw_ptr;
	if ((dir_raw_ptr = opendir(pathname.c_str())) == 0) {
		err_ret("call opendir for {}", pathname);
		return;
	}
	std::shared_ptr<DIR> dp(dir_raw_ptr, [&pathname](DIR* dir) {
		if (closedir(dir) < 0)
			err_ret("call closedir for {}", pathname);
		});

	struct dirent* dirp;

	while ((dirp = readdir(dp.get()))) {
		std::size_t size = pathname.size();
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
			continue;
		struct stat local_st;
		if (lstat(dirp->d_name, &local_st) < 0) {
			err_ret("call stat for {}", pathname);
			continue;
		}
		st = local_st;
		if (S_ISDIR(st.value().st_mode)) {
			if (chdir(dirp->d_name) < 0) {
				err_ret("call chdir for {}", dirp->d_name);
				continue;
			}
			myftw(".", st, nf);
			if (chdir("..") < 0) {
				err_ret("call chdir for {}", dirp->d_name);
				continue;
			}
		}
		else {
			myftw(dirp->d_name, st, nf);
		}
	}

}