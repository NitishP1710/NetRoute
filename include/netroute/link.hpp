#pragma once

#include <string>

namespace netroute {

class Link {
public:
    Link(std::string id,
         std::string source,
         std::string destination,
         double latency_ms,
         double bandwidth_mbps,
         double packet_loss_percent);

    const std::string& id() const noexcept;
    const std::string& source() const noexcept;
    const std::string& destination() const noexcept;

    double latency_ms() const noexcept;
    double bandwidth_mbps() const noexcept;
    double packet_loss_percent() const noexcept;

    bool active() const noexcept;
    void fail() noexcept;
    void recover() noexcept;

private:
    std::string id_;
    std::string source_;
    std::string destination_;
    double latency_ms_;
    double bandwidth_mbps_;
    double packet_loss_percent_;
    bool active_{true};
};

} // namespace netroute
