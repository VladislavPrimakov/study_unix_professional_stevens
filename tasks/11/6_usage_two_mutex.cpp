#include <algorithm>
#include <apue.h>
#include <memory>
#include <pthread.h>
#include <vector>

#define NHASH 29
#define HASH(id) (((unsigned long)id)%NHASH)


class Foo : public std::enable_shared_from_this<Foo> {
	std::size_t  count;
	pthread_mutex_t lock;
	std::size_t id;
	std::shared_ptr<Foo> next;
	static pthread_mutex_t hashlock;
	static std::vector<std::shared_ptr<Foo>> fh;

	Foo(int id) : count(1), id(id) {
		int result = pthread_mutex_init(&lock, NULL);
		if (result != 0) {
			throw std::runtime_error("Failed to initialize mutex, error code: " + std::to_string(result));
		}
		print("Created and inserted to list");
	}
public:
	static std::shared_ptr<Foo> create(int id) {
		auto new_foo = std::shared_ptr<Foo>(new Foo(id));
		int idx = HASH(id);
		pthread_mutex_lock(&hashlock);
		new_foo->next = fh[idx];
		fh[idx] = new_foo;
		pthread_mutex_unlock(&hashlock);
		return new_foo;
	}

	void print(std::string s) {
		std::println("{}: Foo {} hold, count = {}", s, id, count);
	}

	void hold() {
		if (count == 0) {
			throw std::runtime_error("hold() called on object with count 0");
		}
		pthread_mutex_lock(&lock);
		count++;
		pthread_mutex_unlock(&lock);
		print("Increased count");
	}

	static std::shared_ptr<Foo> find(std::size_t id) {
		std::shared_ptr<Foo> fp;
		pthread_mutex_lock(&hashlock);
		for (fp = fh[HASH(id)]; fp != nullptr; fp = fp->next) {
			if (fp->id == id) {
				fp->hold();
				break;
			}
		}
		pthread_mutex_unlock(&hashlock);
		return fp;
	}

	void rele() {
		if (count == 0) {
			throw std::runtime_error("hold() called on object with count 0");
		}
		std::shared_ptr<Foo> tfp;
		int idx;
		pthread_mutex_lock(&lock);
		if (count == 1) { // last reference
			pthread_mutex_unlock(&lock);
			pthread_mutex_lock(&hashlock);
			pthread_mutex_lock(&lock);
			if (count != 1) { // required recheck
				count--;
				pthread_mutex_unlock(&lock);
				pthread_mutex_unlock(&hashlock);
				print("Decreased count");
				return;
			}
			// remove from list
			idx = HASH(id);
			tfp = fh[idx];
			if (tfp.get() == this) {
				fh[idx] = next;
			}
			else {
				while (tfp->next.get() != this) {
					tfp = tfp->next;
				}
				tfp->next = next;
			}
			count--;
			print("Decreased count and removed from list");
			pthread_mutex_unlock(&hashlock);
			pthread_mutex_unlock(&lock);
			pthread_mutex_destroy(&lock);
		}
		else {
			--count;
			pthread_mutex_unlock(&lock);
		}
	}
};

std::vector<std::shared_ptr<Foo>> Foo::fh(NHASH);
pthread_mutex_t Foo::hashlock = PTHREAD_MUTEX_INITIALIZER;

void* foo_thread(void* arg) {
	auto foo = Foo::create(1);
	Foo::find(1);
	foo->rele();
	foo->rele();
	return((void*)0);
}

int main() {
	int err;
	pthread_t ntid;
	err = pthread_create(&ntid, NULL, foo_thread, NULL);
	if (err != 0)
		err_exit(err, "pthread_create");
	foo_thread(NULL);
	pthread_join(ntid, NULL);
	return 0;
}