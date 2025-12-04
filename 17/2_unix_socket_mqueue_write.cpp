#include "apue.h"
#include <liburing.h>
#include <mqueue.h>

constexpr char QUEUE_NAME_FMT[] = "/test_mq_%d";

int main(int argc, char* argv[]) {
	mqd_t mq;
	size_t nbytes;
	unsigned int priority = 0;

	if (argc != 3) {
		err_quit("Usage: {} <queue_index> <message>", argv[0]);
	}

	int queue_idx;
	const char* str_end = argv[1] + std::strlen(argv[1]);
	auto [ptr, ec] = std::from_chars(argv[1], str_end, queue_idx);
	if (ec != std::errc()) {
		err_quit("Invalid queue index: {}", argv[1]);
	}
	std::string name = std::format("/test_mq_{}", queue_idx);

	if ((mq = mq_open(name.c_str(), O_WRONLY)) == (mqd_t)-1) {
		err_sys("mq_open {}", name);
	}

	const char* msg_text = argv[2];
	nbytes = strlen(msg_text);

	if (mq_send(mq, msg_text, nbytes, priority) == -1) {
		err_sys("mq_send");
	}

	mq_close(mq);
	exit(0);
}