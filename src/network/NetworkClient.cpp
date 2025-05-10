// src/network/NetworkClient.cpp
#include "network/NetworkClient.hpp"
#include <iostream>
#include <sstream>

namespace CMQ {

NetworkClient::NetworkClient(const std::string &server_ip, int port, ProtocolType protocol)
    : server_ip_(server_ip), port_(port), protocol_(protocol),
      connected_(false), running_(true), client_fd_(-1) {
#ifdef _WIN32
    WSAStartup(MAKEWORD(2, 2), &wsa_data_);
#endif
    worker_thread_ = std::thread(&NetworkClient::worker_thread, this);
}

NetworkClient::~NetworkClient() {
    disconnect();
    running_ = false;
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

bool NetworkClient::connect_server() {
    client_fd_ = socket(AF_INET,
                        (protocol_ == ProtocolType::TCP) ? SOCK_STREAM : SOCK_DGRAM, 0);
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
    }

    connected_ = true;
    return true;
}

void NetworkClient::disconnect() {
    if (connected_) {
        close_socket(client_fd_);
        connected_ = false;
    }
}

bool NetworkClient::is_connected() const {
    return connected_;
}

void NetworkClient::send_message(const std::string &message) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        task_queue_.emplace([this, message]() {
            if (!connected_) {
                std::cerr << "Not connected to server.\n";
                return;
            }

            if (protocol_ == ProtocolType::TCP) {
                send(client_fd_, message.c_str(), message.size(), 0);
            } else {
                sockaddr_in server_addr{};
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(port_);
                inet_pton(AF_INET, server_ip_.c_str(), &server_addr.sin_addr);

                sendto(client_fd_, message.c_str(), message.size(), 0,
                       (struct sockaddr *)&server_addr, sizeof(server_addr));
            }
        });
        cv_.notify_one();
    }
}

void NetworkClient::receive_message_async() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        task_queue_.emplace([this]() {
            if (!connected_) {
                std::cerr << "Not connected to server.\n";
                return;
            }

            if (protocol_ == ProtocolType::TCP) {
                handle_tcp();
            } else {
                handle_udp();
            }
        });
        cv_.notify_one();
    }
}

// Worker thread that continuously processes tasks in the queue
void NetworkClient::worker_thread() {
    while (running_) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            cv_.wait(lock, [this]() { return !task_queue_.empty() || !running_; });

            if (!running_) break;
            task = std::move(task_queue_.front());
            task_queue_.pop();
        }

        if (task) {
            task();
        }
    }
}

// Handles TCP communication (synchronous receive)
void NetworkClient::handle_tcp() {
    char buffer[1024];
    while (connected_) {
        int bytes = recv(client_fd_, buffer, sizeof(buffer), 0);
        if (bytes > 0) {
            std::cout << "Received (TCP): " << std::string(buffer, bytes) << std::endl;
        } else if (bytes == 0) {
            std::cerr << "Server disconnected.\n";
            disconnect();
            break;
        } else {
            std::cerr << "Failed to receive (TCP).\n";
            disconnect();
            break;
        }
    }
}

// Handles UDP communication (synchronous receive)
void NetworkClient::handle_udp() {
    char buffer[1024];
    sockaddr_in server_addr{};
    socklen_t addr_len = sizeof(server_addr);

    while (connected_) {
        int bytes = recvfrom(client_fd_, buffer, sizeof(buffer), 0,
                             (struct sockaddr *)&server_addr, &addr_len);
        if (bytes > 0) {
            std::cout << "Received (UDP): " << std::string(buffer, bytes) << std::endl;
        } else {
            std::cerr << "Failed to receive (UDP).\n";
            break;
        }
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
