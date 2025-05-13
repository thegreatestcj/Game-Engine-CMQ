// src/gameplay/RateLimiter.cpp
#include "gameplay/RateLimiter.hpp"

namespace CMQ {

    RateLimiter::RateLimiter(int max_requests, double time_window)
        : max_requests_(max_requests), time_window_(time_window) {}

    bool RateLimiter::allow_request(const std::string& client_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();

        auto& record = request_records_[client_id];
        auto& request_count = record.first;
        auto& last_reset = record.second;

        // Reset count if time window has passed
        if (std::chrono::duration<double>(now - last_reset).count() > time_window_) {
            request_count = 0;
            last_reset = now;
        }

        if (request_count < max_requests_) {
            request_count++;
            return true;
        }

        return false; // Rate limit exceeded
    }

}
