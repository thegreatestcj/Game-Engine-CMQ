// src/gameplay/GameServer.cpp
#include "gameplay/GameServer.hpp"
#include <iostream>
#include <sstream>

namespace CMQ {

    GameServer::GameServer(int port, std::shared_ptr<MessageQueue<std::string>> queue, ProtocolType protocol, bool use_ssl)
        : NetworkServer(port, queue, protocol, use_ssl),
          event_bus_(std::make_shared<EventBus>()) {
        register_commands();
        std::cout << "GameServer initialized." << std::endl;
    }

    void GameServer::register_commands() {
        CommandFactory::get_instance().register_all_commands();
        std::cout << "Commands registered.\n";
    }

    void GameServer::handle_player_message(int client_fd, const std::string &message) {
        std::istringstream iss(message);
        std::string command_name, params;
        iss >> command_name;
        std::getline(iss, params);

        auto command = CommandFactory::get_instance().create_command(command_name);
        if (command) {
            command->execute(this, client_fd, params);
        } else {
            send(client_fd, "Unknown command.\n", 17, 0);
        }
    }

    void GameServer::broadcast_message(const std::string& message) {
        std::lock_guard<std::mutex> lock(client_map_mutex_);
        for (const auto& [fd, _] : player_map_) {
            send(fd, message.c_str(), message.size(), 0);
        }
    }

}
