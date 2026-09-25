#include "netroute/routing_strategy.hpp"

namespace netroute {

double BalancedCostStrategy::cost(const Link& link) const {
    // Latency is primary. Packet loss is penalized heavily and
    // low bandwidth adds a small congestion proxy.
    return link.latency_ms()
         + (link.packet_loss_percent() * 2.0)
         + (1000.0 / link.bandwidth_mbps());
}

} // namespace netroute
