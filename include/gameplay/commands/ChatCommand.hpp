// include/gameplay/commands/ChatCommand.hpp
#ifndef CMQ_CHATCOMMAND_HPP
#define CMQ_CHATCOMMAND_HPP

#include "Command.hpp"
#include <string>

namespace CMQ {
    class GameplaySystem; // Forward declaration

    class ChatCommand : public Command {
    public:
        void execute(GameplaySystem* system, int client_id, const std::string& params) override;

    private:
        static bool registered;
    };
}

#endif
