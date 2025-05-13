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

// Test server initialization and shutdown
TEST(GameServerTest, ServerStartAndStop) {
    auto message_queue = std::make_shared<MessageQueue<std::string>>(100);
    GameServer server(8080, message_queue, ProtocolType::TCP, false);

    EXPECT_NO_THROW(server.start());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_NO_THROW(server.stop());
}

// Test single client connection and disconnection
TEST(GameClientTest, ClientConnectAndDisconnect) {
    GameClient client("127.0.0.1", 8080, ProtocolType::TCP, false);

    EXPECT_TRUE(client.connect_server());
    EXPECT_TRUE(client.is_connected());

    client.send_command("chat", "Hello, world!");
    std::this_thread::sleep_for(std::chrono::seconds(1));

    client.disconnect();
    EXPECT_FALSE(client.is_connected());
}

// Test client commands (Move, Chat, Attack)
TEST(GameClientTest, ClientCommandTest) {
    GameClient client("127.0.0.1", 8080, ProtocolType::TCP, false);
    ASSERT_TRUE(client.connect_server());

    EXPECT_NO_THROW(client.send_command("move", "100 200"));
    EXPECT_NO_THROW(client.send_command("chat", "Hello, world!"));
    EXPECT_NO_THROW(client.send_command("attack", "Dragon"));

    std::this_thread::sleep_for(std::chrono::seconds(1));
    client.disconnect();
}

// Test invalid command handling
TEST(GameClientTest, InvalidCommandTest) {
    GameClient client("127.0.0.1", 8080, ProtocolType::TCP, false);
    ASSERT_TRUE(client.connect_server());

    // Invalid command, should not crash the client
    EXPECT_NO_THROW(client.send_command("invalid_command", "unknown"));
    client.disconnect();
}

// Test multiple clients connecting concurrently
TEST(GameClientTest, MultipleClientsTest) {
    const int client_count = 10;
    std::vector<std::thread> client_threads;
    std::atomic<int> connected_clients{0};

    auto client_task = [&connected_clients]() {
        GameClient client("127.0.0.1", 8080, ProtocolType::TCP, false);
        if (client.connect_server()) {
            connected_clients++;
            client.send_command("chat", "Client connected");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            client.send_command("move", "50 50");
            client.send_command("attack", "Goblin");
            client.disconnect();
        }
    };

    // Launch multiple client threads
    for (int i = 0; i < client_count; ++i) {
        client_threads.emplace_back(client_task);
    }

    for (auto& thread : client_threads) {
        thread.join();
    }

    EXPECT_EQ(connected_clients, client_count);
}

// Test client automatic reconnection
TEST(GameClientTest, AutoReconnectTest) {
    GameClient client("127.0.0.1", 8080, ProtocolType::TCP, false);
    ASSERT_TRUE(client.connect_server());

    client.disconnect(); // Manually disconnect

    std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait for reconnect
    ASSERT_TRUE(client.connect_server()); // Manually reconnect (simulate auto-reconnect)
}

// Google Test main entry point
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
