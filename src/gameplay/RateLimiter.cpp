// src/gameplay/RateLimiter.cpp
#include "gameplay/RateLimiter.hpp"

namespace CMQ {

    RateLimiter::RateLimiter(int max_requests, double time_window, size_t max_clients)
        : max_requests_(max_requests), time_window_(time_window),
          max_clients_(max_clients), running_(true) {
        cleanup_thread_ = std::thread(&RateLimiter::cleanup_expired_clients, this);
    }

    RateLimiter::~RateLimiter() {
        running_ = false;
        if (cleanup_thread_.joinable()) {
            cleanup_thread_.join();
        }
    }

    bool RateLimiter::allow_request(const std::string& client_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();

        auto it = request_records_.find(client_id);
        if (it == request_records_.end()) {
            if (request_records_.size() >= max_clients_) {
                cleanup_lru();
            }

            lru_list_.push_front(client_id);
            request_records_[client_id] = {{1, now}, lru_list_.begin()};
            return true;
        }

        auto& [record, lru_it] = it->second;
        auto& request_count = record.first;
        auto& last_reset = record.second;

        lru_list_.erase(lru_it);
        lru_list_.push_front(client_id);
        it->second.second = lru_list_.begin();

        if (std::chrono::duration<double>(now - last_reset).count() > time_window_) {
            request_count = 0;
            last_reset = now;
        }

        if (request_count < max_requests_) {
            request_count++;
            return true;
        }

        return false;
    }

    void RateLimiter::cleanup_expired_clients() {
        using namespace std::chrono;
        while (running_) {
            std::this_thread::sleep_for(seconds(10)); // 每10秒检查

            std::lock_guard<std::mutex> lock(mutex_);
            auto now = steady_clock::now();
            for (auto it = request_records_.begin(); it != request_records_.end();) {
                auto& [record, lru_it] = it->second;
                if (duration<double>(now - record.second).count() > time_window_) {
                    lru_list_.erase(lru_it);
                    it = request_records_.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    void RateLimiter::cleanup_lru() {
        if (!lru_list_.empty()) {
            auto lru_client = lru_list_.back();
            request_records_.erase(lru_client);
            lru_list_.pop_back();
        }
    }

} // namespace CMQ
