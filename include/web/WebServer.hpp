// src/engine/WebServer.hpp
#ifndef CMQ_WEBSERVER_HPP
#define CMQ_WEBSERVER_HPP

#include "engine/MessageQueue.hpp"
#include <thread>
#include <memory>
#include <vector>
#include <set>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace CMQ {

    class WebServer {
    public:
        enum class ProtocolType { TCP, UDP };

        WebServer(int port, std::shared_ptr<MessageQueue<std::string>> queue, ProtocolType protocol);
        ~WebServer();

        void start();
        void stop();

    private:
        void accept_connections();
        void handle_request(int client_fd);
        void handle_udp_request();
        void initialize_ssl();
        void cleanup_ssl();
        void worker_thread();

        int port_;
        int server_fd_;
        int epoll_fd_;
        bool running_;
        ProtocolType protocol_;

        std::thread accept_thread_;
        std::vector<std::thread> worker_threads_;
        std::shared_ptr<MessageQueue<std::string>> message_queue_;

        SSL_CTX *ssl_ctx_; // SSL Context for secure communication

        // Thread pool variables
        std::vector<std::thread> thread_pool_;
        std::mutex task_mutex_;
        std::condition_variable task_cv_;
        std::vector<std::function<void()>> task_queue_;
        bool stop_threads_;

        // Web Status Tracking
        std::atomic<size_t> total_messages_received_;
        std::atomic<size_t> current_queue_size_;

        // WebSocket Management
        std::set<int> websocket_clients_;
        std::mutex websocket_mutex_;
    };

} // namespace CMQ

#endif // CMQ_WEBSERVER_HPP
