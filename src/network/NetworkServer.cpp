// src/network/NetworkServer.cpp
#include "network/NetworkServer.hpp"
#include <iostream>

namespace CMQ {

NetworkServer::NetworkServer(int port, std::shared_ptr<MessageQueue<std::string>> queue, ProtocolType protocol, bool use_ssl)
    : port_(port), protocol_(protocol), running_(false),
      message_queue_(queue), use_ssl_(use_ssl), ssl_ctx_(nullptr),
    dispatcher_(std::shared_ptr<Dispatcher>(&Dispatcher::get_instance(), [](Dispatcher*){})){
#ifdef _WIN32
    WSAStartup(MAKEWORD(2, 2), &wsa_data_);
#endif
    if (use_ssl_) initialize_ssl();
    Dispatcher::get_instance().start(4); // Use Singleton Dispatcher
}

NetworkServer::~NetworkServer() {
    stop();
    cleanup_ssl();
    Dispatcher::get_instance().stop(); // Singleton Dispatcher shutdown
#ifdef _WIN32
    WSACleanup();
#endif
    }

void NetworkServer::start() {
    running_ = true;
    dispatcher_->start();
    accept_thread_ = std::thread(&NetworkServer::accept_connections, this);
    heartbeat_thread_ = std::thread(&NetworkServer::monitor_heartbeat, this);
    std::cout << "NetworkServer started on port " << port_ << std::endl;
}

void NetworkServer::stop() {
    running_ = false;
    std::cout << "[INFO] Stopping NetworkServer..." << std::endl;

    if (server_fd_ >= 0) {
#ifdef _WIN32
        closesocket(server_fd_);
#else
        close(server_fd_);
#endif
        server_fd_ = -1;
    }

    {
        std::lock_guard<std::mutex> lock(client_map_mutex_);
        for (auto& [fd, ssl] : ssl_clients_) {
            if (ssl) {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
#ifdef _WIN32
            closesocket(fd);
#else
            close(fd);
#endif
        }
        ssl_clients_.clear();
        client_heartbeat_.clear();
    }

    if (accept_thread_.joinable()) {
        std::cout << "[INFO] Joining accept_thread_..." << std::endl;
        accept_thread_.join();
        std::cout << "[INFO] accept_thread_ stopped." << std::endl;
    }

    if (heartbeat_thread_.joinable()) {
        std::cout << "[INFO] Joining heartbeat_thread_..." << std::endl;
        heartbeat_thread_.join();
        std::cout << "[INFO] heartbeat_thread_ stopped." << std::endl;
    }

    std::cout << "[INFO] NetworkServer stopped completely." << std::endl;
}


    void NetworkServer::initialize_socket() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        perror("Failed to create socket");
        return;
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
        return;
    }

    listen(server_fd_, 128);

#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(server_fd_, FIONBIO, &mode);
#else
    int flags = fcntl(server_fd_, F_GETFL, 0);
    if (flags < 0) {
        perror("Failed to get socket flags");
        return;
    }
    if (fcntl(server_fd_, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("Failed to set non-blocking mode");
        return;
    }
#endif
}


void NetworkServer::initialize_ssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    ssl_ctx_ = SSL_CTX_new(TLS_server_method());
    if (!ssl_ctx_) {
        std::cerr << "Failed to create SSL context." << std::endl;
    }
}

void NetworkServer::cleanup_ssl() {
    if (ssl_ctx_) {
        SSL_CTX_free(ssl_ctx_);
        EVP_cleanup();
    }
}

    void NetworkServer::accept_connections() {
    std::cout << "[INFO] accept_thread_ starting..." << std::endl;
    initialize_socket();

    while (running_) {
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);

        int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            } else {
                std::cerr << "[ERROR] Accept error: " << strerror(errno) << std::endl;
                break;
            }
        }

        if (!running_) {
            close(client_fd);
            break;
        }

        std::lock_guard<std::mutex> lock(client_map_mutex_);
        client_heartbeat_[client_fd] = std::chrono::steady_clock::now();
        dispatcher_->dispatch([this, client_fd]() {
            handle_client(client_fd);
        });
    }

    std::cout << "[INFO] accept_connections thread exiting..." << std::endl;
}



void NetworkServer::handle_client(int client_fd) {
    SSL* ssl = nullptr;
    if (use_ssl_) {
        ssl = SSL_new(ssl_ctx_);
        SSL_set_fd(ssl, client_fd);
        if (SSL_accept(ssl) <= 0) {
            std::cerr << "SSL handshake failed.\n";
            close_socket(client_fd);
            return;
        }

        std::lock_guard<std::mutex> lock(client_map_mutex_);
        ssl_clients_[client_fd] = ssl;
    }

    char buffer[1024];
    while (running_) {
        int bytes = use_ssl_ ? SSL_read(ssl, buffer, sizeof(buffer)) : recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes > 0) {
            std::string message(buffer, bytes);
            dispatcher_->dispatch([this, client_fd, message]() {
                if (message == "PONG") {
                    std::lock_guard<std::mutex> lock(client_map_mutex_);
                    client_heartbeat_[client_fd] = std::chrono::steady_clock::now();
                } else {
                    handle_task(message);
                }
            });
        } else {
            close_socket(client_fd);
            break;
        }
    }
}

void NetworkServer::monitor_heartbeat() {
    using namespace std::chrono;
    while (running_) {
        std::this_thread::sleep_for(seconds(1));

        auto now = steady_clock::now();
        std::lock_guard<std::mutex> lock(client_map_mutex_);
        for (auto it = client_heartbeat_.begin(); it != client_heartbeat_.end();) {
            if (duration_cast<seconds>(now - it->second).count() > 10) {
                std::cout << "Client " << it->first << " timed out.\n";
                close_socket(it->first);
                it = client_heartbeat_.erase(it);
            } else {
                ++it;
            }
        }
    }

    std::cout << "[INFO] heartbeat_thread_ exiting..." << std::endl;
}


void NetworkServer::handle_task(const std::string &message) {
    if (message == "shutdown") {
        running_ = false;
        std::cout << "[INFO] Server is shutting down..." << std::endl;
        return;
    }
    message_queue_->push(message);
    std::cout << "Message processed: " << message << std::endl;
}

void NetworkServer::close_socket(int fd) {
#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif

    std::lock_guard<std::mutex> lock(client_map_mutex_);
    ssl_clients_.erase(fd);
    client_heartbeat_.erase(fd);
}

} // namespace CMQ
