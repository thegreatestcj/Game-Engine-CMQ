// include/gameplay/GameplaySystem.hpp
#ifndef CMQ_GAMEPLAYSYSTEM_HPP
#define CMQ_GAMEPLAYSYSTEM_HPP

#include "EventBus.hpp"
#include "RateLimiter.hpp"
#include "commands/CommandFactory.hpp"
#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>

namespace CMQ {

    class GameplaySystem {
    public:
        GameplaySystem();
        void initialize();
        void handle_event(const std::string& event_name, const std::string& data);
        void execute_command(const std::string& command_name, const std::string& params, const std::string& client_id);

        // New message methods for commands
        void broadcast_message(const std::string& message);
        void send_message(const std::string& client_id, const std::string& message);

    private:
        std::shared_ptr<EventBus> event_bus_;
        std::shared_ptr<RateLimiter> rate_limiter_;
        std::unordered_map<std::string, std::shared_ptr<Command>> command_registry_;
        std::unordered_map<int, std::string> player_map_; // Player state (client ID -> player name)
        std::mutex player_map_mutex_;
    };

}

#endif
