// src/gameplay/commands/AttackCommand.cpp
#include "gameplay/commands/AttackCommand.hpp"
#include "gameplay/commands/CommandFactory.hpp"
#include "gameplay/GameplaySystem.hpp"
#include <iostream>
#include <sstream>

namespace CMQ {

    // Self-registration
    bool AttackCommand::registered = []() {
        CommandFactory::get_instance().register_command("attack", std::make_shared<AttackCommand>());
        return true;
    }();

    void AttackCommand::execute(GameplaySystem* system, int client_id, const std::string& params) {
        if (system) {
            std::istringstream iss(params);
            std::string target;
            iss >> target;

            if (target.empty()) {
                system->send_message(std::to_string(client_id), "Attack failed: No target specified.");
                return;
            }

            system->broadcast_message("Player " + std::to_string(client_id) + " attacks " + target + "!");
        }
    }

}
