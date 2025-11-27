#include <fstream>
#include <mutex>
#include <string>
#include <ctime>

class LogManager {
private:
    std::mutex log_lock;
    std::string filename;

    std::string timestamp() {
        time_t now = time(nullptr);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return buf;
    }

public:
    LogManager(const std::string &file) : filename(file) {}

    void log(const std::string &msg) {
        std::lock_guard<std::mutex> guard(log_lock);

        std::ofstream out(filename, std::ios::app);
        if (out.is_open()) {
            out << "[" << timestamp() << "] " << msg << "\n";
        }
    }
};
