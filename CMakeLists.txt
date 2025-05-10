cmake_minimum_required(VERSION 3.15)
project(GameEngineCMQ)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include custom CMake helpers
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(Helpers)

# Global include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external
)

# Add subdirectories
add_subdirectory(src/engine)
add_subdirectory(src/gameplay)
add_subdirectory(src/logger)
add_subdirectory(src/config)
add_subdirectory(src/web)
add_subdirectory(src/network)
add_subdirectory(src/game)

# Define the main executable
add_executable(GameEngineCMQ main.cpp)

# Link against module libraries
target_link_libraries(GameEngineCMQ
    CMQEngine
    GameplayModule
    Logger
    Config
    WebView
    GameModule
    Network
)
