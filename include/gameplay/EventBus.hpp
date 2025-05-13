// include/gameplay/EventBus.hpp
#ifndef CMQ_EVENTBUS_HPP
#define CMQ_EVENTBUS_HPP

#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>

namespace CMQ {

    using EventHandler = std::function<void(const std::string&)>;

    class EventBus {
    public:
        // Register an event handler for a specific event
        void register_event(const std::string& event_name, EventHandler handler);

        // Emit an event synchronously (immediate execution)
        void emit_event(const std::string& event_name, const std::string& data);

        // Emit an event asynchronously using a Dispatcher
        void emit_event_async(const std::string& event_name, const std::string& data);

    private:
        std::unordered_map<std::string, std::vector<EventHandler>> handlers_;
        std::mutex mutex_;
    };

}

#endif
