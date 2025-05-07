#include "logger/Logger.hpp"
#include "config/Config.hpp"

int main() {
    Logger::init();
    Config::load("config/config.json");
    Logger::info("[Main] GameEngineCMQ started.");
    return 0;
}
