#ifndef UTILS_THREAD_POOL_DEFINE
#define UTILS_THREAD_POOL_DEFINE

#include "threadpool.h"

// 修改 utils/threadtool/threadpool.cpp 中的 work_thread 函数
void utils::threadpool::work_thread()
{
    // 设置线程为实时调度策略，最高优先级
    struct sched_param sch_params;
    int max_priority = sched_get_priority_max(SCHED_FIFO);
    if (max_priority != -1) {
        sch_params.sched_priority = max_priority;
        if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sch_params) != 0) {
            // 如果设置失败，尝试设置为 RR 调度策略
            sch_params.sched_priority = sched_get_priority_max(SCHED_RR);
            pthread_setschedparam(pthread_self(), SCHED_RR, &sch_params);
        }
    }
    
    for (;;) {
        std::function<void()> task;

        std::unique_lock<std::mutex> lock(this->mutex);

        this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });

        if (this->stop && this->tasks.empty())
            return;

        task = std::move(this->tasks.front());
        this->tasks.pop();

        lock.unlock();

        task();
    }
}

void utils::threadpool::change_thread(size_t count)
{
    kill_thread();
    workers.clear();
    thread_count = count;

    for (size_t i = 0; i < count; ++i)
        workers.emplace_back([this] { work_thread(); });
}

void utils::threadpool::kill_thread()
{
    if (workers.empty())
        return;

    std::unique_lock<std::mutex> lock(mutex);
    stop = true;
    lock.unlock();

    condition.notify_all();

    for (auto &worker : workers) {
        if (worker.joinable())
            worker.join();
    }

    lock.lock();
    stop = false;
    lock.unlock();
}

void utils::threadpool::wait()
{
    kill_thread();
    change_thread(this->thread_count);
}

utils::threadpool::threadpool(size_t count) : thread_count(count), stop(false)
{
    change_thread(this->thread_count);
}

utils::threadpool::~threadpool()
{
    kill_thread();
}

#endif