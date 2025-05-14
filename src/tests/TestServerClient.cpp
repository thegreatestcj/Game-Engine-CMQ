// src/tests/TestServerClient.cpp
#include <gtest/gtest.h>
#include "network/NetworkServer.hpp"
#include "network/NetworkClient.hpp"
#include "engine/Dispatcher.hpp"
#include "gameplay/GameServer.hpp"
#include "gameplay/GameClient.hpp"
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <sstream>

using namespace CMQ;

// Helper function to ensure Dispatcher is reset
void ResetDispatcher() {
    Dispatcher::get_instance().stop();
}

// Test server initialization and shutdown
TEST(GameServerTest, ServerStartAndStop) {
    ResetDispatcher();
    Dispatcher::get_instance().start(4);

    auto message_queue = std::make_shared<MessageQueue<std::string>>(100);
    GameServer server(8080, message_queue, ProtocolType::TCP, false);
    EXPECT_NO_THROW(server.start());

    std::this_thread::sleep_for(std::chrono::seconds(1)); // 确保服务器启动
    EXPECT_NO_THROW(server.stop());
    Dispatcher::get_instance().stop();
}

// Test single client connection and disconnection
TEST(GameClientTest, ClientConnectAndDisconnect) {
    ResetDispatcher();
    Dispatcher::get_instance().start(4);

    auto message_queue = std::make_shared<MessageQueue<std::string>>(100);
    GameServer server(8080, message_queue, ProtocolType::TCP, false);
    server.start();
    std::this_thread::sleep_for(std::chrono::seconds(1)); // 确保服务器启动

    GameClient client("127.0.0.1", 8080, ProtocolType::TCP, false);
    EXPECT_TRUE(client.connect_server());
    EXPECT_TRUE(client.is_connected());

    client.send_command("chat", "Hello, world!");
    std::this_thread::sleep_for(std::chrono::seconds(1));

    client.disconnect();
    EXPECT_FALSE(client.is_connected());

    server.stop();
    Dispatcher::get_instance().stop();
}

// Test client commands (Move, Chat, Attack)
TEST(GameClientTest, ClientCommandTest) {
    ResetDispatcher();
    Dispatcher::get_instance().start(4);

    auto message_queue = std::make_shared<MessageQueue<std::string>>(100);
    GameServer server(8080, message_queue, ProtocolType::TCP, false);
    server.start();
    std::this_thread::sleep_for(std::chrono::seconds(1)); // 确保服务器启动

    GameClient client("127.0.0.1", 8080, ProtocolType::TCP, false);
    ASSERT_TRUE(client.connect_server());
    EXPECT_TRUE(client.is_connected());

    client.send_command("move", "100 200");
    client.send_command("chat", "Hello, world!");
    client.send_command("attack", "Dragon");

    std::this_thread::sleep_for(std::chrono::seconds(1));
    client.disconnect();
    EXPECT_FALSE(client.is_connected());

    server.stop();
    Dispatcher::get_instance().stop();
}

// Google Test main entry point
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
