#ifndef JOB_H
#define JOB_H

#include <string>
#include <functional>

struct Job {
    int client_fd;
    int burst_time;      // simulated job time
    int remaining_time;  // for RR scheduling
    std::function<void()> task;

    Job(int fd, int bt, std::function<void()> fn)
        : client_fd(fd), burst_time(bt), remaining_time(bt), task(fn) {}
};

#endif // JOB_H
