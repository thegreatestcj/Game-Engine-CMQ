// src/engine/Dispatcher.cpp
#include "engine/Dispatcher.hpp"
#include "engine/TaskQueue.hpp"
#include <iostream>

namespace CMQ {

    // Singleton Instance Access
    Dispatcher& Dispatcher::get_instance() {
        static Dispatcher instance;
        return instance;
    }

    // Private constructor for Singleton
    Dispatcher::Dispatcher()
        : task_queue_(std::make_shared<TaskQueue>()), running_(false) {}

    Dispatcher::~Dispatcher() {
        stop();
    }

    void Dispatcher::start(size_t thread_count) {
        if (running_) return;

        running_ = true;
        threads_.reserve(thread_count);
        for (size_t i = 0; i < thread_count; ++i) {
            threads_.emplace_back(&Dispatcher::worker_thread, this);
        }
        std::cout << "Dispatcher started with " << threads_.size() << " threads." << std::endl;
    }

    void Dispatcher::stop() {
        if (!running_) return;

        running_ = false;
        task_queue_->close();

        for (auto& thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        threads_.clear();
        std::cout << "Dispatcher stopped." << std::endl;
    }

    void Dispatcher::dispatch(Task task, bool high_priority) {
        if (!running_) return;
        task_queue_->push(std::move(task), high_priority);
    }

    void Dispatcher::worker_thread() {
        while (running_) {
            Task task;
            if (task_queue_->pop(task)) {
                try {
                    task();
                } catch (const std::exception& e) {
                    std::cerr << "Task execution error: " << e.what() << std::endl;
                }
            }
        }
    }

} // namespace CMQ
