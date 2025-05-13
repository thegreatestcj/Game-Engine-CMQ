# src/gameplay/CMakeLists.txt

# Add GameplayModule using the add_module macro
add_module(GameplayModule
        EventBus.cpp
        GameplaySystem.cpp
        GameServer.cpp
        GameClient.cpp
        RateLimiter.cpp
)

# Automatically include all commands in commands/ directory
file(GLOB COMMAND_SOURCES commands/*.cpp)
target_sources(GameplayModule PRIVATE ${COMMAND_SOURCES})

# SSL Support for secure communication (optional)
find_package(OpenSSL REQUIRED)
target_link_libraries(GameplayModule PUBLIC OpenSSL::SSL OpenSSL::Crypto)
