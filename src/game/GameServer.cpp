// src/game/GameServer.cpp
#include "game/GameServer.hpp"
#include <iostream>
#include <sstream>

namespace CMQ {

    GameServer::GameServer(int port, std::shared_ptr<MessageQueue<std::string>> queue, ProtocolType protocol)
        : NetworkServer(port, queue, protocol) {}

    void GameServer::start_game_server() {
        start(); // Start the underlying NetworkServer
        std::cout << "GameServer started on port " << port_ << std::endl;
    }

    void GameServer::stop_game_server() {
        stop(); // Stop the underlying NetworkServer
        std::cout << "GameServer stopped." << std::endl;
    }

    void GameServer::handle_player_message(int client_fd, const std::string &message) {
        if (message.starts_with("/chat ")) {
            std::string chat_message = message.substr(6);
            broadcast_message("Player " + std::to_string(client_fd) + ": " + chat_message);
        } else if (message.starts_with("/move ")) {
            std::istringstream iss(message.substr(6));
            int x, y;
            iss >> x >> y;
            task_queue_->push([this, client_fd, x, y]() {
                std::lock_guard<std::mutex> lock(player_map_mutex_);
                player_map_[client_fd] = "Moved to " + std::to_string(x) + ", " + std::to_string(y);
            });
        } else if (message.starts_with("/attack ")) {
            std::istringstream iss(message.substr(8));
            int target_fd;
            iss >> target_fd;
            task_queue_->push([this, client_fd, target_fd]() {
                broadcast_message("Player " + std::to_string(client_fd) + " attacks Player " + std::to_string(target_fd));
            });
        } else {
            send(client_fd, "Unknown command.\n", 17, 0);
        }
    }

    void GameServer::broadcast_message(const std::string &message) {
        for (const auto &[fd, _] : player_map_) {
            send(fd, message.c_str(), message.size(), 0);
        }
    }

    void GameServer::process_game_logic() {
        std::cout << "Processing game logic..." << std::endl;
        // Future game logic can be added here
    }

} // namespace CMQ
