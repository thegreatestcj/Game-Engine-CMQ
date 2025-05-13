// include/engine/TaskQueue.hpp
#ifndef CMQ_TASKQUEUE_HPP
#define CMQ_TASKQUEUE_HPP

#include <deque>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace CMQ {
    using Task = std::function<void()>;

    class TaskQueue {
    public:
        TaskQueue();
        ~TaskQueue();

        void push(Task task, bool high_priority = false);
        bool pop(Task& task);
        bool try_pop(Task& task);
        void close();
        bool empty() const;

    private:
        std::deque<Task> queue_;
        mutable std::mutex mutex_;
        std::condition_variable cv_;
        bool closed_;
    };
}

#endif
