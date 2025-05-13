// src/gameplay/GameClient.cpp
#include "gameplay/GameClient.hpp"
#include <iostream>
#include <sstream>

namespace CMQ {

    GameClient::GameClient(const std::string &server_ip, int port, ProtocolType protocol, bool use_ssl)
        : NetworkClient(server_ip, port, protocol, use_ssl) {
        register_commands();
        std::cout << "GameClient initialized.\n";
    }

    void GameClient::register_commands() {
        // Store commands in local registry for direct client-side usage
        command_registry_ = CommandFactory::get_instance().get_registered_commands();
        std::cout << "Commands registered.\n";
    }

    void GameClient::send_command(const std::string &command, const std::string &params) {
        auto it = command_registry_.find(command);
        if (it != command_registry_.end()) {
            std::string message = command + " " + params;
            send_message(message);
        } else {
            std::cerr << "Unknown command: " << command << std::endl;
        }
    }

    void GameClient::receive_message_async() {
        dispatcher_->dispatch([this]() {
            char buffer[1024];
            while (connected_) {
                int bytes = (use_ssl_ ? SSL_read(ssl_, buffer, sizeof(buffer))
                                      : recv(client_fd_, buffer, sizeof(buffer), 0));
                if (bytes > 0) {
                    std::string message(buffer, bytes);
                    std::cout << "Received: " << message << std::endl;
                } else {
                    std::cerr << "Disconnected from server.\n";
                    reconnect();
                    break;
                }
            }
        });
    }

}
