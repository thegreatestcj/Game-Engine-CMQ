// include/network/NetworkClient.hpp
#ifndef CMQ_NETWORK_CLIENT_HPP
#define CMQ_NETWORK_CLIENT_HPP

#include "engine/Dispatcher.hpp"
#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <openssl/ssl.h>
#include <openssl/err.h>

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

    class NetworkClient {
    public:
        NetworkClient(const std::string &server_ip, int port, ProtocolType protocol, bool use_ssl = false);
        ~NetworkClient();

        bool connect_server();
        void disconnect();
        bool is_connected() const;

        void send_message(const std::string &message);
        void receive_message_async();
        void start_heartbeat(); // Start heartbeat mechanism

    protected:
        void handle_tcp();
        void handle_udp();
        void monitor_heartbeat(); // Monitor heartbeat for disconnection
        void reconnect();         // Automatic reconnection
        void initialize_ssl();
        void cleanup_ssl();
        void close_socket(int fd);

        std::string server_ip_;
        int port_;
        int client_fd_;
        ProtocolType protocol_;
        bool use_ssl_;

        std::shared_ptr<Dispatcher> dispatcher_;
        std::atomic<bool> connected_;
        std::atomic<bool> running_;
        std::atomic<bool> heartbeat_active_;
        std::thread heartbeat_thread_;
        std::mutex heartbeat_mutex_;
        std::condition_variable heartbeat_cv_;

        SSL_CTX *ssl_ctx_;  // SSL context for secure communication
        SSL *ssl_;          // SSL object for client

#ifdef _WIN32
        WSADATA wsa_data_;
#endif
    };

}

#endif // CMQ_NETWORK_CLIENT_HPP
