// src/gameplay/GameServer.cpp
#include "gameplay/GameServer.hpp"
#include <iostream>
#include <sstream>

namespace CMQ {

    GameServer::GameServer(int port, std::shared_ptr<MessageQueue<std::string>> queue, ProtocolType protocol, bool use_ssl)
        : NetworkServer(port, queue, protocol, use_ssl),
          gameplay_system_(std::make_shared<GameplaySystem>()) {
        std::cout << "GameServer initialized." << std::endl;
    }

    void GameServer::handle_player_message(int client_fd, const std::string &message) {
        std::istringstream iss(message);
        std::string command_name, params;
        iss >> command_name;
        std::getline(iss, params);

        // Automatically forward command to GameplaySystem (event-driven)
        gameplay_system_->handle_event("player_" + command_name, std::to_string(client_fd) + " " + params);
    }

}
