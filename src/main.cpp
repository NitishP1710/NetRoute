#include "netroute/http_server.hpp"
#include "netroute/network.hpp"
#include "netroute/router.hpp"
#include "netroute/routing_engine.hpp"

#include <cstdlib>
#include <iostream>

using namespace netroute;

namespace {

Network build_demo_network() {
    Network network;

    network.add_router({"R1", "10.0.0.1"});
    network.add_router({"R2", "10.0.0.2"});
    network.add_router({"R3", "10.0.0.3"});
    network.add_router({"R4", "10.0.0.4"});

    network.add_link({"L12", "R1", "R2", 10.0, 1000.0, 0.1});
    network.add_link({"L13", "R1", "R3", 20.0, 500.0, 0.2});
    network.add_link({"L24", "R2", "R4", 15.0, 800.0, 0.1});
    network.add_link({"L34", "R3", "R4", 10.0, 900.0, 0.1});
    network.add_link({"L23", "R2", "R3", 7.0, 600.0, 0.3});

    return network;
}

} // namespace

int main() {
    try {
        auto network = build_demo_network();
        RoutingEngine engine;

        std::uint16_t port = 8080;
        if (const char* value = std::getenv("NETROUTE_PORT")) {
            const auto parsed = std::strtoul(value, nullptr, 10);
            if (parsed > 0 && parsed <= 65535) {
                port = static_cast<std::uint16_t>(parsed);
            }
        }

        HttpServer server(network, engine, port);
        server.run();
    } catch (const std::exception& error) {
        std::cerr << "NetRoute failed: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
