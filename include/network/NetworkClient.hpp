// src/network/NetworkClient.hpp
#ifndef CMQ_NETWORK_CLIENT_HPP
#define CMQ_NETWORK_CLIENT_HPP

#include <string>
#include <memory>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <queue>

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

    using Task = std::function<void()>;

    enum class ProtocolType { TCP, UDP };

    class NetworkClient {
    public:
        NetworkClient(const std::string &server_ip, int port, ProtocolType protocol);
        ~NetworkClient();

        bool connect_server();
        void disconnect();
        bool is_connected() const;

        void send_message(const std::string &message);
        void receive_message_async();

    private:
        void process_tasks();
        void worker_thread();
        void handle_tcp();
        void handle_udp();

        void close_socket(int fd);

        std::string server_ip_;
        int port_;
        ProtocolType protocol_;
        std::atomic<bool> connected_;

        int client_fd_;
        std::thread worker_thread_;
        std::queue<Task> task_queue_;
        std::mutex queue_mutex_;
        std::condition_variable cv_;
        std::atomic<bool> running_;

#ifdef _WIN32
        WSADATA wsa_data_;
#endif
    };

} // namespace CMQ

#endif // CMQ_NETWORK_CLIENT_HPP
