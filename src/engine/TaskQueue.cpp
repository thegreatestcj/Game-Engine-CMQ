// src/engine/TaskQueue.cpp
#include "engine/TaskQueue.hpp"

namespace CMQ {

    TaskQueue::TaskQueue() : closed_(false) {}

    TaskQueue::~TaskQueue() {
        close();
    }

    void TaskQueue::push(Task task, bool high_priority) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (closed_) return;

            if (high_priority) {
                queue_.push_front(std::move(task));  // 高优先级任务插入前端
            } else {
                queue_.push_back(std::move(task));   // 普通任务插入后端
            }
        }
        cv_.notify_one();  // 唤醒一个等待线程
    }

    bool TaskQueue::pop(Task& task) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !queue_.empty() || closed_; });

        if (queue_.empty()) return false;

        task = std::move(queue_.front());
        queue_.pop_front();
        return true;
    }

    bool TaskQueue::try_pop(Task& task) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) return false;

        task = std::move(queue_.front());
        queue_.pop_front();
        return true;
    }

    void TaskQueue::close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
        }
        cv_.notify_all();
    }

    bool TaskQueue::empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

} // namespace CMQ
