// include/gameplay/commands/Command.hpp
#ifndef CMQ_COMMAND_HPP
#define CMQ_COMMAND_HPP

#include <string>
#include <memory>

namespace CMQ {
    class GameServer;

    class Command {
    public:
        virtual ~Command() = default;
        virtual void execute(GameServer* server, int client_id, const std::string& params) = 0;

        // Factory method for automatic registration
        static void register_command(const std::string& name, std::shared_ptr<Command> command);
    };

}

#endif
