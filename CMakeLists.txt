cmake_minimum_required(VERSION 3.15)
project(GameEngineCMQ)

set(CMAKE_CXX_STANDARD 20)
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
add_subdirectory(src/tests)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
        gtest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(gtest)

# Define the main executable
add_executable(GameEngineCMQ main.cpp)

# Link against module libraries
target_link_libraries(GameEngineCMQ
        CMQEngine
        GameplayModule
        Logger
        Config
        WebView
        Network
)

# Link Google Test to your test executable
target_link_libraries(GameEngineCMQ GTest::gtest GTest::gtest_main)
enable_testing()
