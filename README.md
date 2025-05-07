# GameEngineCMQ

High-Performance Concurrent Message Queue for Game Engine

## Overview
This project implements a modular, thread-safe message queue system designed for game engines.

## Build Instructions
```bash
mkdir build && cd build
cmake ..
make -j
```

## Modules
- CMQEngine (core message queue)
- GameplayModule (event bus)
- Logger
- Config
- WebView (asynchronous HTTP server)
