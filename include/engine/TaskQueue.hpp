// src/engine/TaskQueue.hpp
#ifndef CMQ_TASK_QUEUE_HPP
#define CMQ_TASK_QUEUE_HPP

#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace CMQ {

    using Task = std::function<void()>;

    class TaskQueue {
    public:
        TaskQueue();
        ~TaskQueue();

        void push(Task task);

        bool try_pop(Task &task);

        bool pop(Task &task);

        void close();

        bool empty() const;

    private:
        std::queue<Task> queue_;
        mutable std::mutex mutex_;
        std::condition_variable cv_;
        std::atomic<bool> closed_;
    };

} // namespace CMQ

#endif // CMQ_TASK_QUEUE_HPP
