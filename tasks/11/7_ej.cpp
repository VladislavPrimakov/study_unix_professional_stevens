//В маленькой мастерской три рабочих осуществляют окончательную сборку некоторого устройства из
//полуфабриката, установленного в тисках, закрепляя на нем две одинаковые гайки и один винт.Два рабочих умеют
//обращаться только с гаечным ключом, а один — только с отверткой.Действия рабочих схематически описываются
//следующим образом : взять элемент крепежа и, при наличии возможности, установить его на устройство; если все
//три элемента крепежа установлены, то вынуть из тисков готовое устройство и закрепить в них очередной
//полуфабрикат.Размеры устройства позволяют всем рабочим работать одновременно.Используя mutex и два семафора дейкстры, 
// постройте корректную модель сборки устройств с помощью трех потоков : по одному для каждого из рабочих.

#include <apue.h>
#include <memory>
#include <pthread.h>
class Detail {
	friend class Work;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	size_t details_done = 0;
	size_t workers_current = 0;
	size_t details_needed;
	size_t workers_needed;
	size_t workers_finished = 0;
	std::string s_tool;
	std::string s_detail;

public:
	Detail(size_t d_needed, size_t w_needed, const std::string& s_t, const std::string& s_d) : details_needed(d_needed),
		workers_needed(w_needed), s_tool(s_t), s_detail(s_d) {
		pthread_mutex_init(&lock, NULL);
		pthread_cond_init(&cond, NULL);
	}
};

class Work {
	static pthread_mutex_t lock;
	static size_t current_detail_idx;
	static std::vector<std::shared_ptr<Detail>> details;
	size_t worker_id;
public:
	static void Init(const std::vector<std::shared_ptr<Detail>>& d) {
		details = d;
		pthread_mutex_init(&lock, NULL);
		current_detail_idx = 0;
	}

	Work() {
		worker_id = gettid();
		run_work_loop();
	}

	bool try_work(std::shared_ptr<Detail> d) {
		pthread_mutex_lock(&d->lock);
		if (d->details_done >= d->details_needed) {
			pthread_mutex_unlock(&d->lock);
			return false;
		}
		if (d->workers_current < d->workers_needed) {
			d->workers_current++;
			bool is_last = (d->workers_current == d->workers_needed);
			std::println("Worker {} took [{}] for {} ({}/{})",
				worker_id, d->s_tool, d->s_detail, d->workers_current, d->workers_needed);
			if (!is_last) {
				pthread_cond_wait(&d->cond, &d->lock);
			}
			else {
				pthread_cond_broadcast(&d->cond);
			}
			pthread_mutex_unlock(&d->lock);
			usleep(200000); // 200ms
			pthread_mutex_lock(&d->lock);
			d->workers_finished++;
			if (d->workers_finished < d->workers_needed) {
				pthread_cond_wait(&d->cond, &d->lock);
			}
			else {
				d->details_done++;
				std::println(">>> TEAM FINISHED: {} (Progress: {}/{})", d->s_detail, d->details_done, d->details_needed);
				d->workers_current = 0;
				d->workers_finished = 0;
				pthread_cond_broadcast(&d->cond);
			}
			pthread_mutex_unlock(&d->lock);
			return true;
		}
		pthread_mutex_unlock(&d->lock);
		return false;
	}

	void run_work_loop() {
		while (true) {
			pthread_mutex_lock(&lock);
			if (current_detail_idx >= details.size()) {
				pthread_mutex_unlock(&lock);
				break;
			}
			bool detail_ready = true;
			for (auto& d : details) {
				if (d->details_done < d->details_needed) {
					detail_ready = false;
					break;
				}
			}
			if (detail_ready) {
				std::println("\n!!! DETAIL #{} FULLY ASSEMBLED !!!\n", current_detail_idx);
				current_detail_idx++;
				for (auto& d : details) {
					d->details_done = 0;
				}
				pthread_mutex_unlock(&lock);
				continue;
			}
			pthread_mutex_unlock(&lock);
			bool did_work = false;
			for (auto& d_ptr : details) {
				if (try_work(d_ptr)) {
					did_work = true;
					break;
				}
			}
			if (!did_work) {
				usleep(1000);
			}
		}
		std::println("Worker {} finished.", worker_id);
	}
};

pthread_mutex_t Work::lock;
size_t Work::current_detail_idx;
std::vector<std::shared_ptr<Detail>> Work::details;

void* run(void*) { Work w; return nullptr; }

int main() {
	auto bolts = std::make_shared<Detail>(2, 2, "WRENCH", "BOLTS");
	auto screw = std::make_shared<Detail>(1, 1, "SCREWDRIVER", "SCREW");
	std::vector<std::shared_ptr<Detail>> details_list = { bolts, screw };
	Work::Init(details_list);
	const int num_workers = 3;
	std::vector<pthread_t> threads(num_workers);
	for (int i = 0; i < num_workers; ++i) {
		pthread_create(&threads[i], NULL, run, NULL);
	}
	for (int i = 0; i < num_workers; ++i) {
		pthread_join(threads[i], NULL);
	}
	return 0;
}