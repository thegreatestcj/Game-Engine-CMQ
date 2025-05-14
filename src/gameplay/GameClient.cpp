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
        std::cout << "[DEBUG] receive_message_async started." << std::endl;
        while (connected_ && running_) {
            char buffer[1024];
            int bytes = recv(client_fd_, buffer, sizeof(buffer), 0);
            if (bytes > 0) {
                std::string message(buffer, bytes);
                std::cout << "[DEBUG] Client received: " << message << std::endl;
            } else if (bytes == 0) {
                std::cerr << "[DEBUG] Server closed connection.\n";
                break;
            } else {
                std::cerr << "[DEBUG] recv() error: " << strerror(errno) << std::endl;
                break;
            }
        }
        std::cout << "[DEBUG] receive_message_async exiting." << std::endl;
    }




}
