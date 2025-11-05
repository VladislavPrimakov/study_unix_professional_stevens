// example usage of open_max() function from apue.h

import apue;
import std;

int main() {
	long max_files = open_max();
	std::cout << "Maximum open files limit: " << max_files << std::endl;
	std::cout << "Calling again (cached): " << open_max() << std::endl;

	return 0;
}
