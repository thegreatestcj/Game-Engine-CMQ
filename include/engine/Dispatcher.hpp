// include/engine/Dispatcher.hpp
#ifndef CMQ_DISPATCHER_HPP
#define CMQ_DISPATCHER_HPP

#include <deque>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>

namespace CMQ {
    using Task = std::function<void()>;

    class Dispatcher {
    public:
        static Dispatcher& get_instance(); // Singleton access method

        // Deleted copy constructor and assignment operator for Singleton
        Dispatcher(const Dispatcher&) = delete;
        Dispatcher& operator=(const Dispatcher&) = delete;

        void start(size_t thread_count = 4);
        void stop();
        void dispatch(Task task, bool high_priority = false);

    private:
        Dispatcher(); // Private constructor for Singleton
        ~Dispatcher();

        void worker_thread();

        std::shared_ptr<TaskQueue> task_queue_;
        std::vector<std::thread> threads_;
        std::atomic<bool> running_;
    };

}

#endif
