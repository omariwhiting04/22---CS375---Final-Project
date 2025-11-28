#include <mutex>
#include <fstream>
#include <string>

class PerformanceMetrics {
private:
    std::mutex lock;

public:
    long total_jobs = 0;
    long total_burst_time = 0;
    long cache_hits = 0;
    long cache_misses = 0;
    long messages_sent = 0;
    long scheduler_dispatches = 0;

    void log_job(int burst) {
        std::lock_guard<std::mutex> guard(lock);
        total_jobs++;
        total_burst_time += burst;
        scheduler_dispatches++;
    }

    void log_cache_hit() {
        std::lock_guard<std::mutex> guard(lock);
        cache_hits++;
    }

    void log_cache_miss() {
        std::lock_guard<std::mutex> guard(lock);
        cache_misses++;
    }

    void log_message_sent() {
        std::lock_guard<std::mutex> guard(lock);
        messages_sent++;
    }

    void write_report(const std::string &filename) {
        std::lock_guard<std::mutex> guard(lock);
        std::ofstream out(filename);

        out << "==== PERFORMANCE REPORT ====\n";
        out << "Total Jobs: " << total_jobs << "\n";
        out << "Average Burst Time: "
            << (total_jobs == 0 ? 0 : total_burst_time / total_jobs)
            << "\n";
        out << "Scheduler Dispatches: " << scheduler_dispatches << "\n";
        out << "Messages Sent: " << messages_sent << "\n";
        out << "Cache Hits: " << cache_hits << "\n";
        out << "Cache Misses: " << cache_misses << "\n";

        long total_cache = cache_hits + cache_misses;
        out << "Cache Hit Ratio: "
            << (total_cache == 0 ? 0 : (float)cache_hits / total_cache)
            << "\n";

        out << "============================\n";
        out.close();
    }
};
