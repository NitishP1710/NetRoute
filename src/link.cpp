#include "netroute/link.hpp"

#include <stdexcept>
#include <utility>

namespace netroute {

Link::Link(std::string id,
           std::string source,
           std::string destination,
           double latency_ms,
           double bandwidth_mbps,
           double packet_loss_percent)
    : id_(std::move(id)),
      source_(std::move(source)),
      destination_(std::move(destination)),
      latency_ms_(latency_ms),
      bandwidth_mbps_(bandwidth_mbps),
      packet_loss_percent_(packet_loss_percent) {
    if (id_.empty() || source_.empty() || destination_.empty()) {
        throw std::invalid_argument("link id/source/destination cannot be empty");
    }
    if (latency_ms_ < 0.0 || bandwidth_mbps_ <= 0.0 ||
        packet_loss_percent_ < 0.0 || packet_loss_percent_ > 100.0) {
        throw std::invalid_argument("invalid link metrics");
    }
}

const std::string& Link::id() const noexcept { return id_; }
const std::string& Link::source() const noexcept { return source_; }
const std::string& Link::destination() const noexcept { return destination_; }
double Link::latency_ms() const noexcept { return latency_ms_; }
double Link::bandwidth_mbps() const noexcept { return bandwidth_mbps_; }
double Link::packet_loss_percent() const noexcept { return packet_loss_percent_; }
bool Link::active() const noexcept { return active_; }

void Link::fail() noexcept { active_ = false; }
void Link::recover() noexcept { active_ = true; }

} // namespace netroute
