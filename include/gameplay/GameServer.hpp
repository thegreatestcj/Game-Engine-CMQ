// include/gameplay/GameServer.hpp
#ifndef CMQ_GAMESERVER_HPP
#define CMQ_GAMESERVER_HPP

#include "network/NetworkServer.hpp"
#include "gameplay/EventBus.hpp"
#include "gameplay/commands/CommandFactory.hpp"
#include <unordered_map>
#include <memory>
#include <string>

namespace CMQ {

    class GameServer : public NetworkServer {
    public:
        GameServer(int port, std::shared_ptr<MessageQueue<std::string>> queue, ProtocolType protocol, bool use_ssl = false);
        void register_commands();
        void handle_player_message(int client_fd, const std::string &message);
        void broadcast_message(const std::string& message);

    private:
        std::shared_ptr<EventBus> event_bus_;
        std::unordered_map<int, std::string> player_map_; // Player states
    };

}

#endif
