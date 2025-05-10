// src/engine/WebServer.cpp
#include "web/WebServer.hpp"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>

namespace CMQ {

WebServer::WebServer(int port, std::shared_ptr<MessageQueue<std::string>> queue, ProtocolType protocol)
    : port_(port), message_queue_(queue), protocol_(protocol), running_(false), ssl_ctx_(nullptr), stop_threads_(false),
      total_messages_received_(0), current_queue_size_(0) {}

WebServer::~WebServer() {
    stop();
    cleanup_ssl();
}

void WebServer::start() {
    running_ = true;
    accept_thread_ = std::thread(&WebServer::accept_connections, this);
}

void WebServer::stop() {
    running_ = false;
    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }

    for (auto &thread : thread_pool_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void WebServer::accept_connections() {
    // Initialize SSL (if enabled)
    initialize_ssl();

    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd_, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd_, 10);

    while (running_) {
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd_, (struct sockaddr *)&client_addr, &addr_len);

        if (client_fd >= 0) {
            worker_threads_.emplace_back(&WebServer::handle_request, this, client_fd);
        }
    }
}

void WebServer::handle_request(int client_fd) {
    char buffer[1024] = {0};
    read(client_fd, buffer, sizeof(buffer));
    std::string request(buffer);

    if (request.find("GET /status") == 0) {
        std::string status = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{";
        status += "\"total_messages_received\":" + std::to_string(total_messages_received_) + ",";
        status += "\"current_queue_size\":" + std::to_string(current_queue_size_) + "}";
        send(client_fd, status.c_str(), status.size(), 0);
    }

    close(client_fd);
}

void WebServer::initialize_ssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    ssl_ctx_ = SSL_CTX_new(SSLv23_server_method());
}

void WebServer::cleanup_ssl() {
    if (ssl_ctx_) {
        SSL_CTX_free(ssl_ctx_);
    }
    EVP_cleanup();
}

} // namespace CMQ