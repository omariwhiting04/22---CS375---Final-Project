#include "job.h"
#include <queue>
#include <mutex>
#include <condition_variable>

class Scheduler {
private:
    std::queue<Job> rr_queue;
    std::mutex lock;
    std::condition_variable cv;

public:

    void add_job(const Job &job) {
        {
            std::lock_guard<std::mutex> guard(lock);
            rr_queue.push(job);
        }
        cv.notify_one();
    }

    Job get_next_job() {
        std::unique_lock<std::mutex> guard(lock);

        cv.wait(guard, [&] { return !rr_queue.empty(); });

        Job job = rr_queue.front();
        rr_queue.pop();

        return job;  // NO REQUEUE
    }
};
