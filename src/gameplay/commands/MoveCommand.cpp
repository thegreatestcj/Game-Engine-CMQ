// src/gameplay/commands/MoveCommand.cpp
#include "gameplay/commands/MoveCommand.hpp"
#include "gameplay/commands/CommandFactory.hpp"
#include "gameplay/GameplaySystem.hpp"
#include <iostream>
#include <sstream>

namespace CMQ {

    // Self-registration
    bool MoveCommand::registered = []() {
        CommandFactory::get_instance().register_command("move", std::make_shared<MoveCommand>());
        return true;
    }();

    void MoveCommand::execute(GameplaySystem* system, int client_id, const std::string& params) {
        if (system) {
            std::istringstream iss(params);
            int x, y;
            iss >> x >> y;
            system->broadcast_message("Player " + std::to_string(client_id) + " moves to: " + std::to_string(x) + ", " + std::to_string(y));
        }
    }

}
