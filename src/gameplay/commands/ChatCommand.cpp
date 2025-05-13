// src/gameplay/commands/ChatCommand.cpp
#include "gameplay/commands/ChatCommand.hpp"
#include "gameplay/commands/CommandFactory.hpp"
#include "gameplay/GameplaySystem.hpp"
#include <iostream>

namespace CMQ {

    // Self-registration
    bool ChatCommand::registered = []() {
        CommandFactory::get_instance().register_command("chat", std::make_shared<ChatCommand>());
        return true;
    }();

    void ChatCommand::execute(GameplaySystem* system, int client_id, const std::string& params) {
        if (system) {
            system->broadcast_message("Player " + std::to_string(client_id) + ": " + params);
        }
    }

}
