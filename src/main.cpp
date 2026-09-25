#include "netroute/network.hpp"
#include "netroute/router.hpp"
#include "netroute/routing_engine.hpp"

#include <iomanip>
#include <iostream>

using namespace netroute;

namespace {

void print_route(const Route& route) {
    if (!route.reachable()) {
        std::cout << "No route available.\n";
        return;
    }

    for (std::size_t i = 0; i < route.routers.size(); ++i) {
        if (i > 0) {
            std::cout << " -> ";
        }
        std::cout << route.routers[i];
    }

    std::cout << "\n"
              << "Cost: " << std::fixed << std::setprecision(2)
              << route.total_cost << "\n"
              << "Latency: " << route.total_latency_ms << " ms\n"
              << "Min bandwidth: " << route.minimum_bandwidth_mbps << " Mbps\n"
              << "Packet-loss sum: " << route.total_packet_loss_percent << "%\n";
}

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
    auto network = build_demo_network();
    RoutingEngine engine;

    std::cout << "=== NetRoute Demo ===\n\n";
    std::cout << "Initial route R1 -> R4:\n";
    print_route(engine.find_route(network, "R1", "R4"));

    std::cout << "\nFailing link L24...\n";
    network.fail_link("L24");

    std::cout << "Recomputed route R1 -> R4:\n";
    print_route(engine.find_route(network, "R1", "R4"));

    std::cout << "\nRecovering link L24...\n";
    network.recover_link("L24");

    std::cout << "Recomputed route R1 -> R4:\n";
    print_route(engine.find_route(network, "R1", "R4"));
}
