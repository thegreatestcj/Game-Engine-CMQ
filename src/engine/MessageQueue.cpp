// src/engine/MessageQueue.cpp
#include "engine/MessageQueue.hpp"

namespace CMQ {

    template<typename T>
    MessageQueue<T>::MessageQueue(size_t capacity, Comparator comp)
        : capacity_(capacity), closed_(false),
          comparator_(comp ? comp : [](const T&, const T&) { return false; }),
          queue_(comparator_) {}

    template<typename T>
    void MessageQueue<T>::push(const T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return queue_.size() < capacity_ || closed_; });

        if (closed_) return;

        queue_.push(item);
        cv_.notify_one();
    }

    template<typename T>
    bool MessageQueue<T>::pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !queue_.empty() || closed_; });

        if (queue_.empty()) return false;

        item = queue_.top();
        queue_.pop();
        return true;
    }

    template<typename T>
    bool MessageQueue<T>::try_pop(T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) return false;
        item = queue_.top();
        queue_.pop();
        return true;
    }

    template<typename T>
    void MessageQueue<T>::close() {
        closed_ = true;
        cv_.notify_all();
    }

    template<typename T>
    bool MessageQueue<T>::empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    template<typename T>
    size_t MessageQueue<T>::size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    template<typename T>
    bool MessageQueue<T>::is_closed() const {
        return closed_;
    }

    // Explicit instantiation (required for template source file)
    template class MessageQueue<int>; // You can change T to any type used
    template class MessageQueue<std::string>;
}

