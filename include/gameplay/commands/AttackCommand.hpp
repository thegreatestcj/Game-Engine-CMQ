// include/gameplay/commands/AttackCommand.hpp
#ifndef CMQ_ATTACKCOMMAND_HPP
#define CMQ_ATTACKCOMMAND_HPP

#include "Command.hpp"
#include <string>

namespace CMQ {
    class GameplaySystem; // Forward declaration

    class AttackCommand : public Command {
    public:
        void execute(GameplaySystem* system, int client_id, const std::string& params) override;

    private:
        static bool registered;
    };
}

#endif
