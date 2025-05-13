// include/gameplay/GameServer.hpp
#ifndef CMQ_GAMESERVER_HPP
#define CMQ_GAMESERVER_HPP

#include "network/NetworkServer.hpp"
#include "gameplay/GameplaySystem.hpp"
#include <memory>

namespace CMQ {

    class GameServer : public NetworkServer {
    public:
        GameServer(int port, std::shared_ptr<MessageQueue<std::string>> queue, ProtocolType protocol, bool use_ssl = false);
        void handle_player_message(int client_fd, const std::string &message);

    private:
        std::shared_ptr<GameplaySystem> gameplay_system_;
    };

}

#endif
