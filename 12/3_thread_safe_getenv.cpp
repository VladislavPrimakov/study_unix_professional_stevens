#include "apue.h"
#include <pthread.h>

extern char** environ;
pthread_mutex_t env_mutex = PTHREAD_MUTEX_INITIALIZER;;
pthread_once_t init_done = PTHREAD_ONCE_INIT;
pthread_key_t key;

void free_thread_buffer(void* ptr) {
	std::shared_ptr<std::string>* container = static_cast<std::shared_ptr<std::string>*>(ptr);
	delete container;
}

void thread_init(void) {
	pthread_key_create(&key, free_thread_buffer);
}

std::optional<std::shared_ptr<std::string>> getenv_r(const std::string& name) {
	pthread_once(&init_done, thread_init);
	std::shared_ptr<std::string>* container = static_cast<std::shared_ptr<std::string>*>(pthread_getspecific(key));
	if (container == nullptr) {
		container = new std::shared_ptr<std::string>(nullptr);
		pthread_setspecific(key, container);
	}
	pthread_mutex_lock(&env_mutex);
	const char* val = std::getenv(name.c_str());
	if (val == nullptr) {
		pthread_mutex_unlock(&env_mutex);
		return std::nullopt;
	}
	*container = std::make_shared<std::string>(val);
	std::shared_ptr<std::string> result = *container;
	pthread_mutex_unlock(&env_mutex);
	return result;
}


int main() {
	auto value = getenv_r("PATH");
	if (value) {
		std::println("PATH={}", *value.value());
	}
	else {
		std::println("There is no env.PATH");
	}
	return 0;
}