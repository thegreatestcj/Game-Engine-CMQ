// src/engine/TaskQueue.cpp
#include "engine/TaskQueue.hpp"

namespace CMQ {

    TaskQueue::TaskQueue() : closed_(false) {}

    TaskQueue::~TaskQueue() {
        close();
    }

    void TaskQueue::push(Task task) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (closed_) return;
            queue_.push(std::move(task));
        }
        cv_.notify_one();
    }

    bool TaskQueue::try_pop(Task &task) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty() || closed_) return false;

        task = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    bool TaskQueue::pop(Task &task) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !queue_.empty() || closed_; });

        if (queue_.empty()) return false;

        task = std::move(queue_.front());
        queue_.pop();
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
