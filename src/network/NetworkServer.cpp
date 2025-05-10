// src/network/NetworkServer.cpp
#include "network/NetworkServer.hpp"

namespace CMQ {

NetworkServer::NetworkServer(int port, std::shared_ptr<MessageQueue<std::string>> queue, ProtocolType protocol)
    : port_(port), protocol_(protocol), running_(false),
      message_queue_(queue), task_queue_(std::make_shared<TaskQueue>()) {
#ifdef _WIN32
    WSAStartup(MAKEWORD(2, 2), &wsa_data_);
#endif
}

NetworkServer::~NetworkServer() {
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

void NetworkServer::start() {
    running_ = true;
    initialize_socket();
    setup_epoll();

    accept_thread_ = std::thread(&NetworkServer::accept_connections, this);
    for (int i = 0; i < 4; ++i) {
        worker_threads_.emplace_back(&NetworkServer::worker_thread, this);
    }
}

void NetworkServer::stop() {
    running_ = false;
#ifdef _WIN32
    closesocket(server_fd_);
#else
    close(server_fd_);
    close(epoll_fd_);
#endif

    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }

    for (auto &thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    task_queue_->close();
}

bool NetworkServer::is_running() const {
    return running_;
}

void NetworkServer::initialize_socket() {
    server_fd_ = socket(AF_INET, (protocol_ == ProtocolType::TCP) ? SOCK_STREAM : SOCK_DGRAM, 0);
    if (server_fd_ < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);

    int opt = 1;
#ifdef _WIN32
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
#endif

    if (bind(server_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    if (protocol_ == ProtocolType::TCP) {
        listen(server_fd_, 128);
    }
}

void NetworkServer::setup_epoll() {
#ifndef _WIN32
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ < 0) {
        perror("Failed to create epoll instance");
        exit(EXIT_FAILURE);
    }
#endif
}

void NetworkServer::accept_connections() {
    if (protocol_ == ProtocolType::TCP) {
        while (running_) {
            sockaddr_in client_addr{};
            socklen_t addr_len = sizeof(client_addr);
            int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &addr_len);

            if (client_fd >= 0) {
                add_to_epoll(client_fd, EPOLLIN);
                {
                    std::lock_guard<std::mutex> lock(client_map_mutex_);
                    client_map_[client_fd] = client_addr;
                }
            }
        }
    } else {
        handle_udp_client();
    }
}

void NetworkServer::handle_tcp_client(int client_fd) {
    char buffer[1024];
    int bytes = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes > 0) {
        std::string message(buffer, bytes);
        task_queue_->push([this, message]() {
            handle_task(message);
        });
    } else {
        close_socket(client_fd);
    }
}

void NetworkServer::handle_udp_client() {
    char buffer[1024];
    sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);

    while (running_) {
        int bytes = recvfrom(server_fd_, buffer, sizeof(buffer), 0,
                             (struct sockaddr*)&client_addr, &addr_len);
        if (bytes > 0) {
            std::string message(buffer, bytes);
            task_queue_->push([this, message]() {
                handle_task(message);
            });
        }
    }
}

void NetworkServer::worker_thread() {
    Task task;
    while (task_queue_->pop(task)) {
        task();
    }
}

void NetworkServer::handle_task(const std::string &message) {
    message_queue_->push(message);
    std::cout << "Message processed: " << message << std::endl;
}

void NetworkServer::close_socket(int fd) {
#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif
    {
        std::lock_guard<std::mutex> lock(client_map_mutex_);
        client_map_.erase(fd);
    }
}

void NetworkServer::add_to_epoll(int fd, uint32_t events) {
#ifndef _WIN32
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
        perror("Failed to add socket to epoll");
    }
#endif
}

void NetworkServer::remove_from_epoll(int fd) {
#ifndef _WIN32
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
#endif
    close_socket(fd);
}

} // namespace CMQ
