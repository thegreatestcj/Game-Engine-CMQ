// include/gameplay/RateLimiter.hpp
#ifndef CMQ_RATELIMITER_HPP
#define CMQ_RATELIMITER_HPP

#include <unordered_map>
#include <list>
#include <chrono>
#include <mutex>
#include <thread>
#include <string>
#include <atomic>

namespace CMQ {

    class RateLimiter {
    public:
        RateLimiter(int max_requests, double time_window, size_t max_clients = 1000);
        ~RateLimiter();

        bool allow_request(const std::string& client_id);

    private:
        void cleanup_expired_clients();
        void cleanup_lru();

        int max_requests_;
        double time_window_;
        size_t max_clients_;

        using LRUList = std::list<std::string>;
        using ClientRecord = std::pair<int, std::chrono::steady_clock::time_point>;

        std::unordered_map<std::string, std::pair<ClientRecord, LRUList::iterator>> request_records_;
        LRUList lru_list_;

        std::mutex mutex_;
        std::atomic<bool> running_;
        std::thread cleanup_thread_;
    };

} // namespace CMQ

#endif // CMQ_RATELIMITER_HPP
