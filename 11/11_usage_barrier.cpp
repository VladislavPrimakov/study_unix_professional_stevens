// multithreading sort with barrier
#include "apue.h"
#include <algorithm>
#include <chrono>
#include <pthread.h>
#include <random>

class multithread_sort {
	std::vector<int> v;
	pthread_barrier_t b;
	std::size_t num_threads;
	std::size_t num_elements_per_thread;

	struct ThreadArg {
		multithread_sort* instance;
		std::size_t start_idx;
		std::size_t count;
	};

public:
	multithread_sort(std::size_t sz, std::size_t n_threads) : num_threads(n_threads) {
		int err = pthread_barrier_init(&b, NULL, num_threads + 1);
		if (err != 0) {
			throw std::runtime_error("Failed to initialize barrier");
		}
		v.resize(sz);
		num_elements_per_thread = (sz + n_threads - 1) / n_threads;
		if (num_elements_per_thread == 0) num_elements_per_thread = 1;
		std::random_device rd;
		std::default_random_engine generator(rd());
		std::uniform_int_distribution<int> distribution(-100, 100);
		std::generate(v.begin(), v.end(), [&distribution, &generator]() {return distribution(generator); });
	}

	~multithread_sort() {
		pthread_barrier_destroy(&b);
	}

	std::vector<int> get_vector() const {
		return v;
	}

	void sort() {
		std::vector<pthread_t> threads(num_threads);
		std::vector<ThreadArg> args(num_threads);

		for (std::size_t i = 0; i < num_threads; ++i) {
			std::size_t start = i * num_elements_per_thread;
			std::size_t end = std::min(start + num_elements_per_thread, v.size());
			std::size_t count = end - start;
			args[i] = { this, start, count };
			if (count > 0) {
				int err = pthread_create(&threads[i], NULL, &multithread_sort::static_entry_point, &args[i]);
				if (err != 0) {
					throw std::runtime_error("Failed to create thread");
				}
			}
		}
		pthread_barrier_wait(&b);
		for (auto& th : threads) {
			pthread_join(th, NULL);
		}
		merge();
	}

	bool is_sorted() const {
		return std::is_sorted(v.begin(), v.end());
	}
private:
	static void* static_entry_point(void* arg) {
		ThreadArg* t_arg = static_cast<ThreadArg*>(arg);
		t_arg->instance->worker_sort(t_arg->start_idx, t_arg->count);
		return nullptr;
	}

	void worker_sort(std::size_t start_idx, std::size_t count) {
		std::sort(v.begin() + start_idx, v.begin() + start_idx + count);
		pthread_barrier_wait(&b);
	}

	void merge() {
		std::vector<int> result;
		result.reserve(v.size());
		std::vector<std::size_t> current_idx(num_threads);
		std::vector<std::size_t> end_idx(num_threads);
		for (std::size_t i = 0; i < num_threads; ++i) {
			current_idx[i] = i * num_elements_per_thread;
			end_idx[i] = std::min(current_idx[i] + num_elements_per_thread, v.size());
		}
		for (std::size_t k = 0; k < v.size(); ++k) {
			int min_val = std::numeric_limits<int>::max();
			std::size_t min_thread_idx = 0;
			for (std::size_t i = 0; i < num_threads; ++i) {
				if (current_idx[i] < end_idx[i]) {
					int val = v[current_idx[i]];
					if (val < min_val) {
						min_val = val;
						min_thread_idx = i;
					}
				}
			}
			result.push_back(min_val);
			current_idx[min_thread_idx]++;
		}
		v = std::move(result);
	}
};


int main() {
	multithread_sort sorter(8000000, 8);
	std::vector<int> v = sorter.get_vector();
	auto s1 = std::chrono::system_clock::now();
	std::sort(v.begin(), v.end());
	auto e1 = std::chrono::system_clock::now();
	std::println("Elapsed time for single thread: {}", std::chrono::duration<double>(e1 - s1));
	auto s2 = std::chrono::system_clock::now();
	sorter.sort();
	auto e2 = std::chrono::system_clock::now();
	std::println("Elapsed time for 8 threads: {}", std::chrono::duration<double>(e2 - s2));
	std::println("Is vector sorted: {}", sorter.is_sorted());
	return 0;
}