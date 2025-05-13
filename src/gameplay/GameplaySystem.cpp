// src/gameplay/GameplaySystem.cpp
#include "gameplay/GameplaySystem.hpp"
#include <iostream>
#include <sstream>

namespace CMQ {

    GameplaySystem::GameplaySystem()
        : event_bus_(std::make_shared<EventBus>()),
          rate_limiter_(std::make_shared<RateLimiter>(5, 2.0)) {
        initialize();
    }

    void GameplaySystem::initialize() {
        // Automatically detect all commands from CommandFactory
        const auto& commands = CommandFactory::get_instance().get_registered_commands();

        // Automatically register events for each command
        for (const auto& [command_name, _] : commands) {
            event_bus_->register_event("player_" + command_name, [this, command_name](const std::string& data) {
                std::istringstream iss(data);
                std::string client_id, params;
                iss >> client_id;
                std::getline(iss, params);
                execute_command(command_name, params, client_id);
            });
            std::cout << "Registered event: player_" << command_name << std::endl;
        }

        std::cout << "GameplaySystem initialized with " << commands.size() << " commands.\n";
    }

    void GameplaySystem::handle_event(const std::string& event_name, const std::string& data) {
        event_bus_->emit_event(event_name, data);
    }

    void GameplaySystem::execute_command(const std::string& command_name, const std::string& params, const std::string& client_id) {
        if (!rate_limiter_->allow_request(client_id)) {
            std::cerr << "Client " << client_id << " exceeded rate limit.\n";
            return;
        }

        auto command = CommandFactory::get_instance().create_command(command_name);
        if (command) {
            command->execute(this, std::stoi(client_id), params); // Pass GameplaySystem
        } else {
            std::cerr << "Unknown command: " << command_name << std::endl;
        }
    }

    // Broadcast message to all connected players
    void GameplaySystem::broadcast_message(const std::string& message) {
        std::lock_guard<std::mutex> lock(player_map_mutex_);
        for (const auto& [client_id, _] : player_map_) {
            std::cout << "[Broadcast] " << message << std::endl;
        }
    }

    // Send a direct message to a specific player
    void GameplaySystem::send_message(const std::string& client_id, const std::string& message) {
        std::lock_guard<std::mutex> lock(player_map_mutex_);
        std::cout << "[Private] to " << client_id << ": " << message << std::endl;
    }

}
