#pragma once

#include "netroute/network.hpp"
#include "netroute/routing_engine.hpp"

#include <atomic>
#include <cstdint>
#include <string>

namespace netroute {

class HttpServer {
public:
    HttpServer(Network& network, const RoutingEngine& engine, std::uint16_t port);
    void run();
    void stop() noexcept;

private:
    Network& network_;
    const RoutingEngine& engine_;
    std::uint16_t port_;
    std::atomic<bool> running_{true};
};

} // namespace netroute
