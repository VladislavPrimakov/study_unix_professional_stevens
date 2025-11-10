// example usage of sysconf() and pathconf() to print system limits

#include <apue.h>

void pr_sysconf(const std::string& mesg, int name);

void pr_pathconf(const std::string& mesg, const std::string& path, int name);

#define SIZE_NAME 35
#define SIZE_VALUE 25

#define print_compile(name) \
		std::println("{0:<{1}}{2:<{3}}", #name, SIZE_NAME, name, SIZE_VALUE);

#define print_not_defined(name) \
		std::println("{0:<{1}}{2:<{3}}", #name, SIZE_NAME, "(not defined)", SIZE_VALUE);

#define print_sys_conf(name) \
		pr_sysconf(#name, name);

#define print_path_conf(name) \
		pr_pathconf(#name, path, name);


int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::println(std::cerr, "Usage: {} <directory>", argv[0]);
		return 1;
	}
	std::string path = argv[1];
	std::println("{0:<{1}}{2:<{3}}", "Limit Name", SIZE_NAME, "Value", SIZE_VALUE);
#ifdef ARG_MAX
	print_compile(ARG_MAX);
#else
	print_not_defined(ARG_MAX);
#endif
#ifdef _SC_ARG_MAX
	print_sys_conf(_SC_ARG_MAX);
#else
	print_not_defined(_SC_ARG_MAX);
#endif

#ifdef ATEXIT_MAX
	print_compile(ATEXIT_MAX);
#else
	print_not_defined(ATEXIT_MAX);
#endif
#ifdef _SC_ATEXIT_MAX
	print_sys_conf(_SC_ATEXIT_MAX);
#else
	print_not_defined(_SC_ATEXIT_MAX);
#endif

#ifdef CHARCLASS_NAME_MAX
	print_compile(CHARCLASS_NAME_MAX);
#else
	print_not_defined(CHARCLASS_NAME_MAX);
#endif
#ifdef _SC_CHARCLASS_NAME_MAX
	print_sys_conf(_SC_CHARCLASS_NAME_MAX);
#else
	print_not_defined(_SC_CHARCLASS_NAME_MAX);
#endif

#ifdef CHILD_MAX
	print_compile(CHILD_MAX);
#else
	print_not_defined(CHILD_MAX);
#endif
#ifdef _SC_CHILD_MAX
	print_sys_conf(_SC_CHILD_MAX);
#else
	print_not_defined(_SC_CHILD_MAX);
#endif

#ifdef CLK_TCK
	print_compile(CLK_TCK);
#else
	print_not_defined(CLK_TCK);
#endif
#ifdef _SC_CLK_TCK
	print_sys_conf(_SC_CLK_TCK);
#else
	print_not_defined(_SC_CLK_TCK);
#endif

#ifdef COLL_WEIGHTS_MAX
	print_compile(COLL_WEIGHTS_MAX);
#else
	print_not_defined(COLL_WEIGHTS_MAX);
#endif
#ifdef _SC_COLL_WEIGHTS_MAX
	print_sys_conf(_SC_COLL_WEIGHTS_MAX);
#else
	print_not_defined(_SC_COLL_WEIGHTS_MAX);
#endif

#ifdef HOST_NAME_MAX
	print_compile(HOST_NAME_MAX);
#else
	print_not_defined(HOST_NAME_MAX);
#endif
#ifdef _SC_HOST_NAME_MAX
	print_sys_conf(_SC_HOST_NAME_MAX);
#else
	print_not_defined(_SC_HOST_NAME_MAX);
#endif

#ifdef IOV_MAX
	print_compile(IOV_MAX);
#else
	print_not_defined(IOV_MAX);
#endif
#ifdef _SC_IOV_MAX
	print_sys_conf(_SC_IOV_MAX);
#else
	print_not_defined(_SC_IOV_MAX);
#endif

#ifdef LINE_MAX
	print_compile(LINE_MAX);
#else
	print_not_defined(LINE_MAX);
#endif
#ifdef _SC_LINE_MAX
	print_sys_conf(_SC_LINE_MAX);
#else
	print_not_defined(_SC_LINE_MAX);
#endif

#ifdef LOGIN_NAME_MAX
	print_compile(LOGIN_NAME_MAX);
#else
	print_not_defined(LOGIN_NAME_MAX);
#endif
#ifdef _SC_LOGIN_NAME_MAX
	print_sys_conf(_SC_LOGIN_NAME_MAX);
#else
	print_not_defined(_SC_LOGIN_NAME_MAX);
#endif

#ifdef NGROUPS_MAX
	print_compile(NGROUPS_MAX);
#else
	print_not_defined(NGROUPS_MAX);
#endif
#ifdef _SC_NGROUPS_MAX
	print_sys_conf(_SC_NGROUPS_MAX);
#else
	print_not_defined(_SC_NGROUPS_MAX);
#endif

#ifdef OPEN_MAX
	print_compile(OPEN_MAX);
#else
	print_not_defined(OPEN_MAX);
#endif
#ifdef _SC_OPEN_MAX
	print_sys_conf(_SC_OPEN_MAX);
#else
	print_not_defined(_SC_OPEN_MAX);
#endif

#ifdef PAGESIZE
	print_compile(PAGESIZE);
#else
	print_not_defined(PAGESIZE);
#endif
#ifdef PAGE_SIZE
	print_compile(PAGE_SIZE);
#else
	print_not_defined(PAGE_SIZE);
#endif
#ifdef _SC_PAGESIZE
	print_sys_conf(_SC_PAGESIZE);
#else
	print_not_defined(_SC_PAGESIZE);
#endif

#ifdef RE_DUP_MAX
	print_compile(RE_DUP_MAX);
#else
	print_not_defined(RE_DUP_MAX);
#endif
#ifdef _SC_RE_DUP_MAX
	print_sys_conf(_SC_RE_DUP_MAX);
#else
	print_not_defined(_SC_RE_DUP_MAX);
#endif

#ifdef STREAM_MAX
	print_compile(STREAM_MAX);
#else
	print_not_defined(STREAM_MAX);
#endif
#ifdef _SC_STREAM_MAX
	print_sys_conf(_SC_STREAM_MAX);
#else
	print_not_defined(_SC_STREAM_MAX);
#endif

#ifdef SYMLOOP_MAX
	print_compile(SYMLOOP_MAX);
#else
	print_not_defined(SYMLOOP_MAX);
#endif
#ifdef _SC_SYMLOOP_MAX
	print_sys_conf(_SC_SYMLOOP_MAX);
#else
	print_not_defined(_SC_SYMLOOP_MAX);
#endif

#ifdef TTY_NAME_MAX
	print_compile(TTY_NAME_MAX);
#else
	print_not_defined(TTY_NAME_MAX);
#endif
#ifdef _SC_TTY_NAME_MAX
	print_sys_conf(_SC_TTY_NAME_MAX);
#else
	print_not_defined(_SC_TTY_NAME_MAX);
#endif

#ifdef TZNAME_MAX
	print_compile(TZNAME_MAX);
#else
	print_not_defined(TZNAME_MAX);
#endif
#ifdef _SC_TZNAME_MAX
	print_sys_conf(_SC_TZNAME_MAX);
#else
	print_not_defined(_SC_TZNAME_MAX);
#endif
#ifdef _PC_FILESIZEBITS
	print_path_conf(_PC_FILESIZEBITS);
#else
	print_not_defined(_PC_FILESIZEBITS);
#endif

#ifdef LINK_MAX
	print_compile(LINK_MAX);
#else
	print_not_defined(LINK_MAX);
#endif
#ifdef _PC_LINK_MAX
	print_path_conf(_PC_LINK_MAX);
#else
	print_not_defined(_PC_LINK_MAX);
#endif

#ifdef MAX_CANON
	print_compile(MAX_CANON);
#else
	print_not_defined(MAX_CANON);
#endif
#ifdef _PC_MAX_CANON
	print_path_conf(_PC_MAX_CANON);
#else
	print_not_defined(_PC_MAX_CANON);
#endif

#ifdef MAX_INPUT
	print_compile(MAX_INPUT);
#else
	print_not_defined(MAX_INPUT);
#endif
#ifdef _PC_MAX_INPUT
	print_path_conf(_PC_MAX_INPUT);
#else
	print_not_defined(_PC_MAX_INPUT);
#endif

#ifdef NAME_MAX
	print_compile(NAME_MAX);
#else
	print_not_defined(NAME_MAX);
#endif
#ifdef _PC_NAME_MAX
	print_path_conf(_PC_NAME_MAX);
#else
	print_not_defined(_PC_NAME_MAX);
#endif

#ifdef PATH_MAX
	print_compile(PATH_MAX);
#else
	print_not_defined(PATH_MAX);
#endif
#ifdef _PC_PATH_MAX
	print_path_conf(_PC_PATH_MAX);
#else
	print_not_defined(_PC_PATH_MAX);
#endif

#ifdef PIPE_BUF
	print_compile(PIPE_BUF);
#else
	print_not_defined(PIPE_BUF);
#endif
#ifdef _PC_PIPE_BUF
	print_path_conf(_PC_PIPE_BUF);
#else
	print_not_defined(_PC_PIPE_BUF);
#endif

#ifdef SYMLINK_MAX
	print_compile(SYMLINK_MAX);
#else
	print_not_defined(SYMLINK_MAX);
#endif
#ifdef _PC_SYMLINK_MAX
	print_path_conf(_PC_SYMLINK_MAX);
#else
	print_not_defined(_PC_SYMLINK_MAX);
#endif
	return 0;
}


void pr_sysconf(const std::string& mesg, int name) {
	long val;
	std::print("{0:<{1}}", mesg, SIZE_NAME);
	errno = 0;
	if ((val = sysconf(name)) < 0) {
		if (errno != 0) {
			if (errno == EINVAL)
				std::println("(not supported)");
			else
				err_ret("sysconf error");
		}
		else {
			std::println("(not limit)");
		}
	}
	else {
		std::println("{0:<{1}}", val, SIZE_VALUE);
	}
}

void pr_pathconf(const std::string& mesg, const std::string& path, int name) {
	long val;
	std::print("{0:<{1}}", mesg, SIZE_NAME);
	errno = 0;
	if ((val = pathconf(path.c_str(), name)) < 0) {
		if (errno != 0) {
			if (errno == EINVAL)
				std::println("(not supported)");
			else
				err_ret("pathconf error for path " + path);
		}
		else {
			std::println("(not limit)");
		}
	}
	else {
		std::println("{0:<{1}}", val, SIZE_VALUE);
	}
}
