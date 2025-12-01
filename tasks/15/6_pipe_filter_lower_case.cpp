#include "apue.h"
#include <algorithm>

int main() {
	std::string s;
	s.resize(MAXLINE);
	while (fgets(s.data(), s.size(), stdin) != nullptr) {
		std::transform(s.begin(), s.end(), s.begin(), [](unsigned char& c) { return std::tolower(c); });
		if (fputs(s.c_str(), stdout) == EOF)
			err_sys("output error");
	}
	if (ferror(stdin))
		err_sys("input error");
	exit(0);
}