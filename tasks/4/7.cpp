// recursive tree file search

#include  <apue.h>
#include <dirent.h>
#include <filesystem>
#include <new>

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
void myftw(std::string& pathname, nfiles& nf);

int main(int argc, char* argv[]) {
	try {
		if (argc != 2)
			err_quit("Using {}: ftw <base_dir>", argv[0]);
		nfiles nf;
		std::string s;
		s.reserve(1024);
		s = argv[1];
		myftw(s, nf);
		nf.printStat();
	}
	catch (const std::bad_alloc& e) {
		err_quit("{}", e.what());
	}
	return 0;
}

void myftw(std::string& pathname, nfiles& nf) {
	struct stat statbuf;

	if (lstat(pathname.c_str(), &statbuf) < 0) {
		err_ret("call stat for {}", pathname);
		return;
	}

	if (!S_ISDIR(statbuf.st_mode)) {
		switch (statbuf.st_mode & S_IFMT) {
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
	DIR* dp;
	if ((dp = opendir(pathname.c_str())) == 0) {
		err_ret("call opendir for {}", pathname);
		return;
	}
	struct dirent* dirp;
	while ((dirp = readdir(dp))) {
		std::size_t size = pathname.size();
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
			continue;
		while (pathname.capacity() + strlen(dirp->d_name) < pathname.capacity()) {
			pathname.reserve(pathname.capacity() * 2);
		}
		pathname.append("/");
		pathname.append(dirp->d_name);
		myftw(pathname, nf);
		pathname.resize(size);
	}
	if (closedir(dp) < 0)
		err_ret("call closedir for {}", pathname);
}