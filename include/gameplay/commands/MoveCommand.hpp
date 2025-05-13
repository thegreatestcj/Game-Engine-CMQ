// include/gameplay/commands/MoveCommand.hpp
#ifndef CMQ_MOVECOMMAND_HPP
#define CMQ_MOVECOMMAND_HPP

#include "Command.hpp"
#include <string>

namespace CMQ {
    class MoveCommand : public Command {
    public:
        void execute(GameServer* server, int client_id, const std::string& params) override;

    private:
        static bool registered;
    };
}

#endif
