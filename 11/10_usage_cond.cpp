#include "apue.h"
#include <pthread.h>
#include <queue>

struct msg {
	int id;
	std::string data;
};

class message_process {
	pthread_cond_t ready;
	pthread_mutex_t lock;
	std::queue<msg> messages;
public:
	message_process() {
		int err = pthread_cond_init(&ready, NULL);
		if (err != 0)
			throw std::runtime_error("pthread_cond_init failed: " + std::to_string(err));
		err = pthread_mutex_init(&lock, NULL);
		if (err != 0)
			throw std::runtime_error("pthread_mutex_init failed: " + std::to_string(err));
		ready = PTHREAD_COND_INITIALIZER;
		lock = PTHREAD_MUTEX_INITIALIZER;
	}
	message_process(const message_process&) = delete;
	message_process& operator=(const message_process&) = delete;
	~message_process() {
		pthread_cond_destroy(&ready);
		pthread_mutex_destroy(&lock);
	}

	void process_msg() {
		while (true) {
			pthread_mutex_lock(&lock);
			while (messages.empty()) {
				pthread_cond_wait(&ready, &lock);
			}
			msg mp = messages.front();
			messages.pop();
			pthread_mutex_unlock(&lock);
			std::cout << "Processing msg ID: " << mp.id << ", Data: " << mp.data << std::endl;
			if (mp.id == -1) break;
		}
	}

	void enqueue_msg(const msg& mp) {
		pthread_mutex_lock(&lock);
		messages.push(mp);
		pthread_mutex_unlock(&lock);
		pthread_cond_signal(&ready);
	}
};

void* consumer_thread(void* arg) {
	message_process* mp = static_cast<message_process*>(arg);
	mp->process_msg();
	return nullptr;
}

int main() {
	message_process mp;
	pthread_t tid;

	pthread_create(&tid, NULL, consumer_thread, &mp);

	for (int i = 0; i < 5; ++i) {
		msg m = { i, "Hello " + std::to_string(i) };
		std::println("Enqueuing msg ID: {}", i);
		mp.enqueue_msg(m);
		sleep(1);
	}

	mp.enqueue_msg({ -1, "Stop" });
	pthread_join(tid, NULL);

	return 0;
}