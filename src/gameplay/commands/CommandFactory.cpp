// src/gameplay/commands/CommandFactory.cpp
#include "gameplay/commands/CommandFactory.hpp"
#include <iostream>

namespace CMQ {

    CommandFactory& CommandFactory::get_instance() {
        static CommandFactory instance;
        return instance;
    }

    std::shared_ptr<Command> CommandFactory::create_command(const std::string& command_name) {
        auto it = command_registry_.find(command_name);
        if (it != command_registry_.end()) {
            return it->second;
        }
        return nullptr;
    }

    void CommandFactory::register_command(const std::string& name, std::shared_ptr<Command> command) {
        command_registry_[name] = std::move(command);
        std::cout << "Registered command: " << name << std::endl;
    }

    const std::unordered_map<std::string, std::shared_ptr<Command>>& CommandFactory::get_registered_commands() const {
        return command_registry_;
    }

}
