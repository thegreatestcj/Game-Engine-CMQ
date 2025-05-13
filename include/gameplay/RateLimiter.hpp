// include/gameplay/RateLimiter.hpp
#ifndef CMQ_RATELIMITER_HPP
#define CMQ_RATELIMITER_HPP

#include <chrono>
#include <mutex>
#include <unordered_map>
#include <string>

namespace CMQ {

    class RateLimiter {
    public:
        // Constructor: max_requests = allowed requests, time_window = seconds
        RateLimiter(int max_requests = 5, double time_window = 2.0);

        // Checks if the client is allowed to make a request
        bool allow_request(const std::string& client_id);

    private:
        int max_requests_;
        double time_window_; // Time window in seconds

        // Stores request count and last reset time for each client
        std::unordered_map<std::string, std::pair<int, std::chrono::steady_clock::time_point>> request_records_;
        std::mutex mutex_;
    };

}

#endif
