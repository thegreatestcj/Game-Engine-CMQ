// src/gameplay/commands/ChatCommand.cpp
#include "gameplay/commands/ChatCommand.hpp"
#include "gameplay/commands/CommandFactory.hpp"
#include "gameplay/GameServer.hpp"
#include <iostream>

namespace CMQ {

    // Self-registration
    bool ChatCommand::registered = []() {
        CommandFactory::get_instance().register_command("chat", std::make_shared<ChatCommand>());
        return true;
    }();

    void ChatCommand::execute(GameServer* server, int client_id, const std::string& params) {
        if (server) {
            server->broadcast_message("Player " + std::to_string(client_id) + ": " + params);
        }
    }

}
