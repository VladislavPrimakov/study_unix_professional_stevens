#include "apue.h"
#include <pthread.h>

class Foo {
	int count;
	pthread_mutex_t lock;
	int id;

public:
	Foo(int id) : count(1), id(id) {
		int result = pthread_mutex_init(&lock, NULL);
		if (result != 0) {
			throw std::runtime_error("Failed to initialize mutex, error code: " + std::to_string(result));
		}
		print();
	}

	~Foo() {
		pthread_mutex_destroy(&lock);
	}

	void print() {
		std::println("Foo {} hold, count = {}", id, count);
	}

	void hold() {
		if (count == 0) {
			throw std::runtime_error("hold() called on object with count 0");
		}
		pthread_mutex_lock(&lock);
		count++;
		pthread_mutex_unlock(&lock);
		print();
	}

	void rele() {
		if (count == 0) {
			throw std::runtime_error("hold() called on object with count 0");
		}
		pthread_mutex_lock(&lock);
		if (--count == 0) {
			pthread_mutex_unlock(&lock);
			pthread_mutex_destroy(&lock);
		}
		else {
			pthread_mutex_unlock(&lock);
		}
		print();
	}
};

int main() {
	Foo* foo = new Foo(1);
	foo->hold();
	foo->rele();
	foo->rele();
	foo->rele();

	return 0;
}