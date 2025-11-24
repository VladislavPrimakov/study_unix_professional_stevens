#include <apue.h>
#include <deque>
#include <memory>
#include <pthread.h>




template <typename T>
class deque_rwlock {
	pthread_rwlock_t lock;
	std::deque<std::shared_ptr<T>> job_queue;
public:
	deque_rwlock() {
		int err = pthread_rwlock_init(&lock, NULL);
		if (err != 0)
			throw std::runtime_error("pthread_rwlock_init failed: " + std::to_string(err));
	}
	deque_rwlock(const deque_rwlock&) = delete;

	deque_rwlock& operator=(const deque_rwlock&) = delete;

	~deque_rwlock() {
		pthread_rwlock_destroy(&lock);
	}

	void push_front(std::shared_ptr<T>& job) {
		pthread_rwlock_wrlock(&lock);
		job_queue.push_front(job);
		pthread_rwlock_unlock(&lock);
	}

	void push_back(std::shared_ptr<T>& job) {
		pthread_rwlock_wrlock(&lock);
		job_queue.push_back(job);
		pthread_rwlock_unlock(&lock);
	}

	void remove(std::shared_ptr<T>& job) {
		pthread_rwlock_wrlock(&lock);
		auto it = std::find(job_queue.begin(), job_queue.end(), job);
		if (it != job_queue.end()) {
			job_queue.erase(it);
		}
		pthread_rwlock_unlock(&lock);
	}

	std::shared_ptr<T> find(pthread_t id) {
		pthread_rwlock_rdlock(&lock);
		for (const auto& job : job_queue) {
			if (pthread_equal(job->id, id)) {
				pthread_rwlock_unlock(&lock);
				return job;
			}
		}
		pthread_rwlock_unlock(&lock);
		return nullptr;
	}

	std::size_t size() {
		pthread_rwlock_rdlock(&lock);
		std::size_t sz = job_queue.size();
		pthread_rwlock_unlock(&lock);
		return sz;
	}
};

class job {
	friend class deque_rwlock<job>;
	pthread_t id;
public:
	job(pthread_t id) : id(id) {}
};

deque_rwlock<job> global_queue;

void* worker(void* arg) {
	pthread_t my_id = pthread_self();
	auto my_job = std::make_shared<job>(my_id);
	std::println("Thread {} adding job...", my_id);
	global_queue.push_back(my_job);
	sleep(1); // 1s
	global_queue.remove(my_job);
	return nullptr;
}

int main() {
	const int NUM_THREADS = 4;
	std::vector<pthread_t> threads(NUM_THREADS);
	for (int i = 0; i < NUM_THREADS; ++i) {
		pthread_create(&threads[i], NULL, worker, NULL);
	}
	usleep(500000); // 0.5s

	std::println("--- Main thread checking queue ---");
	std::println("Queue size: {}", global_queue.size());
	pthread_t target_id = threads[0];
	auto found_job = global_queue.find(target_id);
	if (found_job) {
		std::println("Main thread found job for thread {} !", target_id);
	}
	else {
		std::println("Main thread did NOT find job for thread {}", target_id);
	}
	std::println("--- Waiting for threads to finish ---");

	for (int i = 0; i < NUM_THREADS; ++i) {
		pthread_join(threads[i], NULL);
	}

	std::println("Final queue size: {}", global_queue.size());
	return 0;
}