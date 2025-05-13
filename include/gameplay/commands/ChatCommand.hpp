// include/gameplay/commands/ChatCommand.hpp
#ifndef CMQ_CHATCOMMAND_HPP
#define CMQ_CHATCOMMAND_HPP

#include "Command.hpp"
#include <string>

namespace CMQ {
    class ChatCommand : public Command {
    public:
        void execute(GameServer* server, int client_id, const std::string& params) override;

    private:
        // Self-registration for automatic command registration
        static bool registered;
    };
}

#endif
