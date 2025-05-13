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
        // Register all commands using CommandFactory
        CommandFactory::get_instance().register_all_commands();
        command_registry_ = CommandFactory::get_instance().get_registered_commands();

        // Automatically register events for each command
        for (const auto& [command_name, _] : command_registry_) {
            // Register an event for each command
            event_bus_->register_event("player_" + command_name, [this, command_name](const std::string& data) {
                std::istringstream iss(data);
                std::string client_id, params;
                iss >> client_id;
                std::getline(iss, params);
                execute_command(command_name, params, client_id);
            });

            std::cout << "Registered event: player_" << command_name << std::endl;
        }

        std::cout << "GameplaySystem initialized with " << command_registry_.size() << " commands.\n";
    }

    void GameplaySystem::handle_event(const std::string& event_name, const std::string& data) {
        event_bus_->emit_event(event_name, data);
    }

    void GameplaySystem::execute_command(const std::string& command_name, const std::string& params, const std::string& client_id) {
        if (!rate_limiter_->allow_request(client_id)) {
            std::cerr << "Client " << client_id << " exceeded rate limit.\n";
            return;
        }

        auto it = command_registry_.find(command_name);
        if (it != command_registry_.end()) {
            auto command = it->second;
            command->execute(nullptr, 0, params);
        } else {
            std::cerr << "Unknown command: " << command_name << std::endl;
        }
    }

}
