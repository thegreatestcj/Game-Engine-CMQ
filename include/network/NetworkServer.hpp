// include/network/NetworkServer.hpp
#ifndef CMQ_NETWORK_SERVER_HPP
#define CMQ_NETWORK_SERVER_HPP

#include "engine/Dispatcher.hpp"
#include "engine/MessageQueue.hpp"
#include <memory>
#include <thread>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <chrono>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

namespace CMQ {

    enum class ProtocolType { TCP, UDP };

    class NetworkServer {
    public:
        NetworkServer(int port, std::shared_ptr<MessageQueue<std::string>> queue, ProtocolType protocol, bool use_ssl = false);
        ~NetworkServer();

        void start();
        void stop();
        bool is_running() const;

    protected:
        void initialize_socket();
        void initialize_ssl();
        void cleanup_ssl();
        void accept_connections();
        void handle_client(int client_fd);
        void handle_task(const std::string &message);
        void monitor_heartbeat(); // Monitor client heartbeats
        void close_socket(int fd);

        int port_;
        int server_fd_;
        ProtocolType protocol_;
        bool use_ssl_;
        std::shared_ptr<MessageQueue<std::string>> message_queue_;
        std::shared_ptr<Dispatcher> dispatcher_;

        std::thread accept_thread_;
        std::thread heartbeat_thread_;
        std::unordered_map<int, std::chrono::steady_clock::time_point> client_heartbeat_;
        std::unordered_map<int, SSL*> ssl_clients_;
        std::mutex client_map_mutex_;
        std::atomic<bool> running_;

        SSL_CTX *ssl_ctx_; // SSL Context for secure communication

#ifdef _WIN32
        WSADATA wsa_data_;
#endif
    };

}

#endif
