// output file type information in arguments

import apue;
import std;
#include <sys/stat.h>

int main(int argc, char* argv[]) {
    struct stat buf{};
    std::string ptr;
    for (std::size_t i = 1; i < argc; i++) {
        std::print("{}: ", argv[i]);
        if (lstat(argv[i], &buf) < 0) {
            err_ret("lstat");
            continue;
        }
        if (S_ISREG(buf.st_mode))
            ptr = "regular file";
        else if (S_ISDIR(buf.st_mode))
            ptr = "directory";
        else if (S_ISCHR(buf.st_mode))
            ptr = "symbolic device";
        else if (S_ISBLK(buf.st_mode))
            ptr = "block device";
        else if (S_ISFIFO(buf.st_mode))
            ptr = "fifo";
        else if (S_ISLNK(buf.st_mode))
            ptr = "symlink";
        else if (S_ISSOCK(buf.st_mode))
            ptr = "socket";
        else
            ptr = "** unknow file type **";
        std::println("{}", ptr);
    }
    return 0;
}
