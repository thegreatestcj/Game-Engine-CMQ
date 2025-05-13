// main.cpp (Project Root)
#include "network/NetworkServer.hpp"
#include "engine/Dispatcher.hpp"
#include "gameplay/GameServer.hpp"
#include <iostream>
#include <thread>
#include <atomic>
#include <csignal>

using namespace CMQ;

// Global flag for clean shutdown
std::atomic<bool> running{true};

// Signal handler for graceful shutdown
void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        running = false;
        std::cout << "\n[INFO] Signal received. Shutting down..." << std::endl;
    }
}

int main() {
    // Register signal handler for Ctrl+C
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // Initialize Dispatcher (thread pool)
    std::cout << "[INFO] Starting Dispatcher with 4 threads..." << std::endl;
    Dispatcher::get_instance().start(4);

    // Set up the Game Server
    std::cout << "[INFO] Initializing Game Server on port 8080..." << std::endl;
    auto message_queue = std::make_shared<MessageQueue<std::string>>(100);
    GameServer server(8080, message_queue, ProtocolType::TCP, false);

    // Start the Game Server
    server.start();
    std::cout << "[INFO] Game Server started. Waiting for players..." << std::endl;

    // Main server loop (keep running until interrupted)
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Graceful shutdown
    std::cout << "[INFO] Shutting down Game Server..." << std::endl;
    server.stop();
    Dispatcher::get_instance().stop();
    std::cout << "[INFO] Server and Dispatcher stopped gracefully.\n";

    return 0;
}
