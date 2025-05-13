// include/gameplay/commands/CommandFactory.hpp
#ifndef CMQ_COMMAND_FACTORY_HPP
#define CMQ_COMMAND_FACTORY_HPP

#include <unordered_map>
#include <memory>
#include <string>
#include "Command.hpp"

namespace CMQ {
    class CommandFactory {
    public:
        static CommandFactory& get_instance();
        std::shared_ptr<Command> create_command(const std::string& command_name);
        const std::unordered_map<std::string, std::shared_ptr<Command>>& get_registered_commands() const;

        // Register a command (called by each Command subclass)
        void register_command(const std::string& name, std::shared_ptr<Command> command);

    private:
        CommandFactory() = default;
        std::unordered_map<std::string, std::shared_ptr<Command>> command_registry_;
    };
}

#endif
