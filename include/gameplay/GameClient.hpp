// include/gameplay/GameClient.hpp
#ifndef CMQ_GAMECLIENT_HPP
#define CMQ_GAMECLIENT_HPP

#include "network/NetworkClient.hpp"
#include "gameplay/commands/CommandFactory.hpp"
#include <memory>
#include <unordered_map>

namespace CMQ {

    class GameClient : public NetworkClient {
    public:
        GameClient(const std::string &server_ip, int port, ProtocolType protocol, bool use_ssl = false);

        void register_commands();
        void send_command(const std::string &command, const std::string &params);
        void receive_message_async();

    private:
        std::unordered_map<std::string, std::shared_ptr<Command>> command_registry_;
    };

}

#endif
