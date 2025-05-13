// src/gameplay/EventBus.cpp
#include "gameplay/EventBus.hpp"
#include "engine/Dispatcher.hpp"

namespace CMQ {

    void EventBus::register_event(const std::string& event_name, EventHandler handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        handlers_[event_name].emplace_back(std::move(handler));
    }

    void EventBus::emit_event(const std::string& event_name, const std::string& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = handlers_.find(event_name);
        if (it != handlers_.end()) {
            for (const auto& handler : it->second) {
                handler(data);
            }
        }
    }

    void EventBus::emit_event_async(const std::string& event_name, const std::string& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = handlers_.find(event_name);
        if (it != handlers_.end()) {
            for (const auto& handler : it->second) {
                // Use Singleton Dispatcher for async task execution
                Dispatcher::get_instance().dispatch([handler, data]() {
                    handler(data);
                });
            }
        }
    }

}
