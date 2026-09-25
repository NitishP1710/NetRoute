#pragma once

#include <string>
#include <vector>

namespace netroute {

struct Route {
    std::vector<std::string> routers;
    double total_cost{0.0};
    double total_latency_ms{0.0};
    double minimum_bandwidth_mbps{0.0};
    double total_packet_loss_percent{0.0};

    bool reachable() const noexcept {
        return !routers.empty();
    }
};

} // namespace netroute
