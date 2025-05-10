// src/network/NetworkServer.hpp
#ifndef CMQ_NETWORK_SERVER_HPP
#define CMQ_NETWORK_SERVER_HPP

#include "engine/MessageQueue.hpp"
#include "engine/TaskQueue.hpp"
#include <thread>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <unordered_map>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <mutex>
#include <condition_variable>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#endif

namespace CMQ {

    using Task = std::function<void()>;

    enum class ProtocolType { TCP, UDP };

    class NetworkServer {
    public:
        NetworkServer(int port, std::shared_ptr<MessageQueue<std::string>> queue, ProtocolType protocol);
        virtual ~NetworkServer();

        void start();
        void stop();
        bool is_running() const;

    protected:
        // Task processing function, can be overridden in derived classes (e.g., GameServer)
        virtual void handle_task(const std::string& message);

    private:
        void initialize_socket();
        void accept_connections();
        void handle_tcp_client(int client_fd);
        void handle_udp_client();
        void worker_thread();
        void setup_epoll();

        // Utility functions
        void close_socket(int fd);
        void add_to_epoll(int fd, uint32_t events);
        void remove_from_epoll(int fd);

        int port_;
        int server_fd_;
        int epoll_fd_;
        ProtocolType protocol_;
        std::atomic<bool> running_;
        std::thread accept_thread_;
        std::vector<std::thread> worker_threads_;
        std::shared_ptr<MessageQueue<std::string>> message_queue_;
        std::shared_ptr<TaskQueue> task_queue_;

        std::unordered_map<int, sockaddr_in> client_map_;
        std::mutex client_map_mutex_;

#ifdef _WIN32
        WSADATA wsa_data_;
#endif
    };

} // namespace CMQ

#endif // CMQ_NETWORK_SERVER_HPP
