// src/game/GameServer.hpp
#ifndef CMQ_GAMESERVER_HPP
#define CMQ_GAMESERVER_HPP

#include "network/NetworkServer.hpp"
#include <unordered_map>
#include <string>
#include <memory>

namespace CMQ {

    class GameServer : public NetworkServer {
    public:
        GameServer(int port, std::shared_ptr<MessageQueue<std::string>> queue, ProtocolType protocol);
        void start_game_server();
        void stop_game_server();

    protected:
        void handle_player_message(int client_fd, const std::string &message);
        void broadcast_message(const std::string &message);
        void process_game_logic();

    private:
        std::unordered_map<int, std::string> player_map_; // Player ID -> Player Name
        std::mutex player_map_mutex_; // Mutex for player map thread safety
    };

} // namespace CMQ

#endif // CMQ_GAMESERVER_HPP
