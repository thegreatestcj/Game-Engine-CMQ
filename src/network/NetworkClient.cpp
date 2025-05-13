// src/network/NetworkClient.cpp
#include "network/NetworkClient.hpp"
#include <iostream>
#include <sstream>
#include <chrono>

namespace CMQ {

NetworkClient::NetworkClient(const std::string &server_ip, int port, ProtocolType protocol, bool use_ssl)
    : server_ip_(server_ip), port_(port), protocol_(protocol),
      use_ssl_(use_ssl), connected_(false), running_(true),
      heartbeat_active_(false), ssl_ctx_(nullptr), ssl_(nullptr) {
#ifdef _WIN32
    WSAStartup(MAKEWORD(2, 2), &wsa_data_);
#endif
    if (use_ssl_) initialize_ssl();
    Dispatcher::get_instance().start(2); // Use Singleton Dispatcher
}

NetworkClient::~NetworkClient() {
    disconnect();
    cleanup_ssl();
    Dispatcher::get_instance().stop(); // Singleton Dispatcher shutdown
#ifdef _WIN32
    WSACleanup();
#endif
}

bool NetworkClient::connect_server() {
    client_fd_ = socket(AF_INET, (protocol_ == ProtocolType::TCP) ? SOCK_STREAM : SOCK_DGRAM, 0);
    if (client_fd_ < 0) {
        std::cerr << "Failed to create socket.\n";
        return false;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    inet_pton(AF_INET, server_ip_.c_str(), &server_addr.sin_addr);

    if (protocol_ == ProtocolType::TCP) {
        if (connect(client_fd_, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Failed to connect to TCP server.\n";
            close_socket(client_fd_);
            return false;
        }

        // Initialize SSL connection if enabled
        if (use_ssl_) {
            ssl_ = SSL_new(ssl_ctx_);
            SSL_set_fd(ssl_, client_fd_);
            if (SSL_connect(ssl_) <= 0) {
                std::cerr << "SSL handshake failed.\n";
                close_socket(client_fd_);
                return false;
            }
        }
    }

    connected_ = true;
    std::cout << "Connected to server: " << server_ip_ << ":" << port_ << std::endl;
    start_heartbeat();
    return true;
}

void NetworkClient::disconnect() {
    running_ = false;
    connected_ = false;
    heartbeat_active_ = false;
    heartbeat_cv_.notify_all();

    if (heartbeat_thread_.joinable()) {
        heartbeat_thread_.join();
    }

    close_socket(client_fd_);
    if (ssl_) {
        SSL_free(ssl_);
        ssl_ = nullptr;
    }
}

bool NetworkClient::is_connected() const {
    return connected_;
}

void NetworkClient::send_message(const std::string &message) {
    if (!connected_) {
        std::cerr << "Not connected to server.\n";
        return;
    }

    dispatcher_->dispatch([this, message]() {
        if (use_ssl_) {
            SSL_write(ssl_, message.c_str(), message.size());
        } else {
            send(client_fd_, message.c_str(), message.size(), 0);
        }
    });
}

void NetworkClient::receive_message_async() {
    if (!connected_) {
        std::cerr << "Not connected to server.\n";
        return;
    }

    dispatcher_->dispatch([this]() {
        if (protocol_ == ProtocolType::TCP) {
            handle_tcp();
        } else {
            handle_udp();
        }
    });
}

void NetworkClient::handle_tcp() {
    char buffer[1024];
    while (connected_) {
        int bytes = use_ssl_ ? SSL_read(ssl_, buffer, sizeof(buffer)) : recv(client_fd_, buffer, sizeof(buffer), 0);
        if (bytes > 0) {
            std::string msg(buffer, bytes);
            std::cout << "Received: " << msg << std::endl;
            if (msg == "HEARTBEAT") {
                send_message("PONG");
            }
        } else {
            std::cerr << "Disconnected from server.\n";
            reconnect();
            break;
        }
    }
}

void NetworkClient::handle_udp() {
    char buffer[1024];
    sockaddr_in server_addr{};
    socklen_t addr_len = sizeof(server_addr);

    while (connected_) {
        int bytes = recvfrom(client_fd_, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, &addr_len);
        if (bytes > 0) {
            std::cout << "Received (UDP): " << std::string(buffer, bytes) << std::endl;
        }
    }
}

void NetworkClient::start_heartbeat() {
    heartbeat_active_ = true;
    heartbeat_thread_ = std::thread(&NetworkClient::monitor_heartbeat, this);
}

void NetworkClient::monitor_heartbeat() {
    using namespace std::chrono_literals;
    while (heartbeat_active_) {
        std::this_thread::sleep_for(5s);
        if (connected_) {
            send_message("HEARTBEAT");
        }
    }
}

void NetworkClient::reconnect() {
    disconnect();
    std::cout << "Attempting to reconnect...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Reconnect delay

    while (!connected_ && running_) {
        std::cout << "Reconnecting...\n";
        if (connect_server()) {
            std::cout << "Reconnected to server.\n";
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void NetworkClient::initialize_ssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    ssl_ctx_ = SSL_CTX_new(TLS_client_method());
}

void NetworkClient::cleanup_ssl() {
    if (ssl_ctx_) {
        SSL_CTX_free(ssl_ctx_);
        EVP_cleanup();
    }
}

void NetworkClient::close_socket(int fd) {
#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif
}

} // namespace CMQ
