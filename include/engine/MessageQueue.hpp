// include/engine/MessageQueue.hpp
#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <vector>

namespace CMQ {

    template<typename T>
    class MessageQueue {
    public:
        using Comparator = std::function<bool(const T&, const T&)>;

        // Constructor: optional priority comparator
        explicit MessageQueue(size_t capacity, Comparator comp = nullptr);

        void push(const T& item);
        bool pop(T& item);       // blocking
        bool try_pop(T& item);   // non-blocking

        void close();            // stop the queue
        bool empty() const;
        size_t size() const;
        bool is_closed() const;

    private:
        struct DefaultCompare {
            bool operator()(const T& a, const T& b) const {
                return false; // FIFO if no comparator provided
            }
        };

        std::priority_queue<T, std::vector<T>, Comparator> queue_;
        mutable std::mutex mutex_;
        std::condition_variable cv_;
        std::atomic<bool> closed_;
        size_t capacity_;
        Comparator comparator_;
    };

} // namespace CMQ
